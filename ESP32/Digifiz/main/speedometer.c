#include "speedometer.h"
#include "setup.h"
#include "params.h"
#include "esp_log.h"
#include "millis.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define TAG "spd_m"

static QueueHandle_t spd_evt_queue = NULL;
static volatile uint64_t last_time_rising = 0;
static volatile uint64_t last_time_falling = 0;
static volatile uint64_t interval = 0;
static volatile uint32_t last_speed_value_millis = 0;
static volatile uint8_t spd_cnt = 0;
static gptimer_handle_t spd_gptimer = 0;
static uint32_t average_intv_q = 0;
static uint32_t buf_intv_q = 0;

pcnt_unit_handle_t spd_m_pcnt_unit;

uint32_t mSpdData = 0;

static volatile uint32_t spd_isr_count = 0;
static volatile uint32_t spd_received_frames = 0;
static volatile uint32_t spd_last_logged_millis = 0;

typedef struct {
    int data[SPD_WINDOW_SIZE];
    int index;
} CircularBufferSPD;

CircularBufferSPD spd_buffer;

// Initialize the circular buffer
static void init_spd_buffer(CircularBufferSPD *buffer) {
    for (int i = 0; i < SPD_WINDOW_SIZE; i++) {
        buffer->data[i] = 0;
    }
    buffer->index = 0;
}

// Insert a new value into the circular buffer
static void insert_spd_buffer(CircularBufferSPD *buffer, int value) {
    buffer->data[buffer->index] = value;
    buffer->index = (buffer->index + 1) % SPD_WINDOW_SIZE;
}

// Find the median of the buffer's data
static int find_spd_median(CircularBufferSPD *buffer) {
    int sorted[SPD_WINDOW_SIZE];
    for (int i = 0; i < SPD_WINDOW_SIZE; i++) {
        sorted[i] = buffer->data[i];
    }
    // Insertion sort
    for (int i = 1; i < SPD_WINDOW_SIZE; i++) {
        int key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j = j - 1;
        }
        sorted[j + 1] = key;
    }
    return sorted[SPD_WINDOW_SIZE / 2];
}

// Median filter function
static int spd_median_filter(CircularBufferSPD *buffer, int new_value) {
    insert_spd_buffer(buffer, new_value);
    return find_spd_median(buffer);
}

static void IRAM_ATTR spd_gpio_isr_handler(void* arg) {
    uint64_t current_time;
    uint32_t intv_q = 0;
    spd_isr_count++;  // Count every ISR call

    gptimer_get_raw_count(spd_gptimer, &current_time);
    uint32_t current_gpio_level = gpio_get_level(SPD_M_PIN);
    if (current_gpio_level == 1) {
        if (digifiz_parameters.stable_speed_input.value) {
            // Rising edge, stable input
            if ((current_time-last_time_falling) > SPD_DEBOUNCE_TICKS) {
                interval = current_time - last_time_rising;
            }
            last_time_rising = current_time;
            intv_q = (uint32_t)interval;
            spd_cnt++;
            if (spd_cnt % 10) {
                if (intv_q>0)
                xQueueSendFromISR(spd_evt_queue, &intv_q, NULL);
            }
        } else {
            // Rising edge, filtered input
            if ((last_time_falling - current_time) > SPD_DEBOUNCE_TICKS) {
                interval = current_time - last_time_rising;
            }
            last_time_rising = current_time;
            intv_q = (uint32_t)interval;
            average_intv_q += intv_q;
            spd_cnt++;
            if ((spd_cnt % 8) == 0) {
                buf_intv_q = average_intv_q >> 3;
                average_intv_q = 0;
                if (buf_intv_q>0)
                    xQueueSendFromISR(spd_evt_queue, &buf_intv_q, NULL);
            }
        }
    } else {
        // Falling edge
        last_time_falling = current_time;
    }
}

static void spd_timer_init() {
    ESP_LOGI(TAG, "Create timer handle");
    spd_gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &spd_gptimer));
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(spd_gptimer));
    ESP_ERROR_CHECK(gptimer_start(spd_gptimer));
}

static void spd_gpio_init() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << SPD_M_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_isr_handler_add(SPD_M_PIN, spd_gpio_isr_handler, (void*) SPD_M_PIN);
}

static void spd_frequency_task(void* arg) {
    uint64_t intv = 0;
    uint32_t spd_sense = 0;
    while (1) {
        if (xQueueReceive(spd_evt_queue, &intv, portMAX_DELAY)) {
            spd_received_frames++;  // Count each received item
            if (intv>0)
            {
                if (digifiz_parameters.stable_speed_input.value) {
                // Stable input
                uint32_t spd = 1000000 / intv;
                if (spd < 999) { // Sanity check
                    mSpdData = spd;
                    last_speed_value_millis = millis();
                }
                } else {
                    // Filtered input
                    spd_sense = spd_median_filter(&spd_buffer, 1000000 / intv);
                    if (spd_sense < 1200) {
                        mSpdData = spd_sense;
                        last_speed_value_millis = millis();
                    }
                }
            }
        }
    }
}

// For backward compatibility: direct frequency callback (current version)
static void spd_m_callback(double hz) {
    mSpdData = (uint32_t)fabs(hz);
    ESP_LOGI(TAG, "SPD_M: %f Hz", hz);
    last_speed_value_millis = millis();
}

void initSpeedometer() {
    ESP_LOGI(TAG, "initSpeedometer started");
    mSpdData = 0;
    last_speed_value_millis = millis();

    if (digifiz_parameters.stable_speed_input.value || digifiz_parameters.filtered_speed_input.value) {
        // New ISR-based logic
        init_spd_buffer(&spd_buffer);
        spd_evt_queue = xQueueCreate(10, sizeof(uint32_t));
        spd_timer_init();
        spd_gpio_init();
        xTaskCreatePinnedToCore(spd_frequency_task, "spd_count_task", 2048, NULL, 6, NULL, 1);
    } else {
        // Current version: frequency_count_task_function
        frequency_count_configuration_t * config = malloc(sizeof(*config));
        config->pcnt_gpio = SPD_M_PIN;
        config->pcnt_unit = &spd_m_pcnt_unit;
        config->sampling_period_seconds = ((float)digifiz_parameters.speed_meas_period.value) / 1000.0f;
        config->sampling_window_seconds = ((float)digifiz_parameters.speed_meas_window.value) / 1000.0f;
        config->frequency_update_callback = &spd_m_callback;
        xTaskCreatePinnedToCore(&frequency_count_task_function, "spd_m_count_task", 4096, config, 6, NULL, 1);
    }
    ESP_LOGI(TAG, "initSpeedometer ended");
}

uint32_t readLastSpeed() {
    uint32_t now = millis();

    // if ((now - spd_last_logged_millis) > 1000) {
    //     spd_last_logged_millis = now;

    //     printf("Debug: Frames: %lu, ISR calls: %lu",
    //             spd_received_frames, spd_isr_count);

    //     spd_received_frames = 0;
    //     spd_isr_count = 0;
    // }

    if ((millis()>2000)&&((millis() - last_speed_value_millis) < 1000)) {
        return mSpdData;
    } else {
        return 0;
    }
}