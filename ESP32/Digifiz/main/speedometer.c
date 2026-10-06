#include "speedometer.h"
#include "params.h"
#include "millis.h"
#include "pulse_filters.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include <assert.h>
#include <stdlib.h>

#define TAG "spd_m"

static QueueHandle_t spd_evt_queue;
static speed_pulse_filter_t speed_capture;
static speed_median_filter_t speed_filter;
static bool speed_filtered;
static uint32_t speed_read_timeout_ms = 1000u;
static volatile uint32_t last_speed_value_millis;
pcnt_unit_handle_t spd_m_pcnt_unit;
uint32_t mSpdData;

typedef struct {
    uint32_t period_us;
    uint32_t measured_ms;
} speed_sample_t;

static void IRAM_ATTR spd_gpio_isr_handler(void *arg)
{
    speed_sample_t sample;
    if (!speed_pulse_edge(&speed_capture, gpio_get_level(SPD_M_PIN) != 0,
                          (uint64_t)esp_timer_get_time(), SPD_DEBOUNCE_TICKS,
                          &sample.period_us)) {
        return;
    }
    sample.measured_ms = (uint32_t)(xTaskGetTickCountFromISR() * portTICK_PERIOD_MS);
    BaseType_t task_woken = pdFALSE;
    // Capture EVERY qualified pulse. A busy task receives the latest period,
    // rather than silently dropping it when an old FIFO becomes full.
    xQueueOverwriteFromISR(spd_evt_queue, &sample, &task_woken);
    if (task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void spd_gpio_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << SPD_M_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    speed_pulse_init(&speed_capture, gpio_get_level(SPD_M_PIN) != 0,
                     (uint64_t)esp_timer_get_time());
    ESP_ERROR_CHECK(gpio_isr_handler_add(SPD_M_PIN, spd_gpio_isr_handler, NULL));
}

static void spd_frequency_task(void *arg)
{
    speed_sample_t sample;
    uint32_t previous_ms = 0;
    bool have_previous = false;
    while (1) {
        if (!xQueueReceive(spd_evt_queue, &sample, portMAX_DELAY)) {
            continue;
        }
        if ((uint32_t)(millis() - sample.measured_ms) >= 1000u) {
            continue; // Delayed delivery must not revive an expired reading.
        }
        // Reject implausible periods BEFORE they enter the median window.
        uint32_t limit = speed_filtered ? 1200u : 999u;
        if (sample.period_us == 0 || 1000000u / sample.period_us >= limit) {
            continue;
        }
        if (!have_previous || (uint32_t)(sample.measured_ms - previous_ms) >= 1000u) {
            speed_filter = (speed_median_filter_t){0};
        }
        previous_ms = sample.measured_ms;
        have_previous = true;
        uint32_t period_us = speed_filtered
            ? speed_median_period(&speed_filter, sample.period_us)
            : sample.period_us;
        mSpdData = 1000000u / period_us;
        last_speed_value_millis = sample.measured_ms;
    }
}

static void spd_m_callback(double hz)
{
    mSpdData = (uint32_t)fabs(hz);
    last_speed_value_millis = millis();
}

void initSpeedometer(void)
{
    ESP_LOGI(TAG, "initSpeedometer started");
    mSpdData = 0;
    last_speed_value_millis = millis();
    speed_read_timeout_ms = 1000u;
    // Input mode, GPIO handler and filtering policy are selected on boot.
    speed_filtered = !digifiz_parameters.stable_speed_input.value
                     && digifiz_parameters.filtered_speed_input.value;
    if (digifiz_parameters.stable_speed_input.value || speed_filtered) {
        speed_filter = (speed_median_filter_t){0};
        spd_evt_queue = xQueueCreate(1, sizeof(speed_sample_t));
        assert(spd_evt_queue != NULL);
        spd_gpio_init();
        BaseType_t created = xTaskCreatePinnedToCore(spd_frequency_task,
            "spd_count_task", 2048, NULL, 6, NULL, 1);
        assert(created == pdPASS);
    } else {
        frequency_count_configuration_t *config = calloc(1, sizeof(*config));
        assert(config != NULL);
        config->pcnt_gpio = SPD_M_PIN;
        config->pcnt_unit = &spd_m_pcnt_unit;
        config->sampling_period_seconds = (float)digifiz_parameters.speed_meas_period.value / 1000.0f;
        config->sampling_window_seconds = (float)digifiz_parameters.speed_meas_window.value / 1000.0f;
        // PCNT updates can be two seconds apart; the one-second ISR timeout
        // must not blank a valid count between those scheduled updates.
        uint32_t counter_timeout = digifiz_parameters.speed_meas_period.value * 2u;
        if (counter_timeout > speed_read_timeout_ms) {
            speed_read_timeout_ms = counter_timeout;
        }
        config->filter_length = SPD_M_FILTER_LENGTH;
        config->frequency_update_callback = spd_m_callback;
        BaseType_t created = xTaskCreatePinnedToCore(frequency_count_task_function,
            "spd_m_count_task", 4096, config, 6, NULL, 1);
        assert(created == pdPASS);
    }
    ESP_LOGI(TAG, "initSpeedometer ended");
}

uint32_t readLastSpeed(void)
{
    return (uint32_t)(millis() - last_speed_value_millis) < speed_read_timeout_ms ? mSpdData : 0;
}
