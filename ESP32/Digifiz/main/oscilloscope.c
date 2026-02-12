#include "oscilloscope.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "tacho.h"
#include "speedometer.h"

#define OSC_MAGIC 0x43534F44U /* DOSC */
#define OSC_VERSION 1U

static const char *TAG = "osc";

static uint32_t rpm_words[OSC_PACKED_WORDS];
static uint32_t speed_words[OSC_PACKED_WORDS];
static volatile uint32_t write_index = 0;
static volatile uint32_t wrapped = 0;
static portMUX_TYPE osc_mux = portMUX_INITIALIZER_UNLOCKED;
static esp_timer_handle_t osc_timer = NULL;

static inline void set_sample_bit(uint32_t *words, uint32_t sample_idx, bool level)
{
    const uint32_t word_idx = sample_idx / 32U;
    const uint32_t bit_idx = sample_idx % 32U;
    const uint32_t bit_mask = (1UL << bit_idx);

    words[word_idx] &= ~bit_mask;
    if (level) {
        words[word_idx] |= bit_mask;
    }
}

static void oscilloscope_sample_cb(void *arg)
{
    (void)arg;

    const uint32_t idx = write_index;
    const bool rpm_level = gpio_get_level(RPM_PIN) != 0;
    const bool speed_level = gpio_get_level(SPD_M_PIN) != 0;

    portENTER_CRITICAL(&osc_mux);
    set_sample_bit(rpm_words, idx, rpm_level);
    set_sample_bit(speed_words, idx, speed_level);

    write_index = idx + 1U;
    if (write_index >= OSC_SAMPLE_COUNT) {
        write_index = 0;
        wrapped = 1U;
    }
    portEXIT_CRITICAL(&osc_mux);
}

void oscilloscope_init(void)
{
    memset(rpm_words, 0, sizeof(rpm_words));
    memset(speed_words, 0, sizeof(speed_words));
    write_index = 0;
    wrapped = 0;

    const esp_timer_create_args_t args = {
        .callback = &oscilloscope_sample_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "osc_sampler"
    };

    ESP_ERROR_CHECK(esp_timer_create(&args, &osc_timer));
    const uint64_t period_us = 1000000ULL / OSC_SAMPLE_RATE_HZ;
    ESP_ERROR_CHECK(esp_timer_start_periodic(osc_timer, period_us));
    ESP_LOGI(TAG, "Oscilloscope sampler started: %u Hz, %u samples", OSC_SAMPLE_RATE_HZ, OSC_SAMPLE_COUNT);
}

void oscilloscope_deinit(void)
{
    if (osc_timer == NULL) {
        return;
    }

    ESP_ERROR_CHECK(esp_timer_stop(osc_timer));
    ESP_ERROR_CHECK(esp_timer_delete(osc_timer));
    osc_timer = NULL;
}

esp_err_t oscilloscope_build_dump(uint8_t **out_buf, size_t *out_size)
{
    if (out_buf == NULL || out_size == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const size_t payload_size = sizeof(oscilloscope_dump_header_t) + sizeof(rpm_words) + sizeof(speed_words);
    uint8_t *payload = malloc(payload_size);
    if (payload == NULL) {
        return ESP_ERR_NO_MEM;
    }

    oscilloscope_dump_header_t header = {
        .magic = OSC_MAGIC,
        .version = OSC_VERSION,
        .sample_rate_hz = OSC_SAMPLE_RATE_HZ,
        .sample_count = OSC_SAMPLE_COUNT,
        .packed_words = OSC_PACKED_WORDS,
        .write_index = 0,
        .wrapped = 0
    };

    portENTER_CRITICAL(&osc_mux);
    header.write_index = write_index;
    header.wrapped = wrapped;
    memcpy(payload, &header, sizeof(header));
    memcpy(payload + sizeof(header), rpm_words, sizeof(rpm_words));
    memcpy(payload + sizeof(header) + sizeof(rpm_words), speed_words, sizeof(speed_words));
    portEXIT_CRITICAL(&osc_mux);

    *out_buf = payload;
    *out_size = payload_size;
    return ESP_OK;
}
