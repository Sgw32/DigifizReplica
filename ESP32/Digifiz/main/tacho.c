#include "tacho.h"
#include "setup.h"
#include "esp_log.h"
#include "millis.h"
#include "params.h"

uint32_t mRPMSenseData;

#define TAG "tacho"
#define RPM_ALGORITHM_LEGACY 0u
#define RPM_ALGORITHM_STABLE 1u
#define RPM_ALGORITHM_GLITCH 2u
#define RPM_MIN_LOW_US_DEFAULT 100u
#define RPM_MIN_HIGH_US_DEFAULT 100u

static QueueHandle_t gpio_evt_queue = NULL;
static volatile uint64_t last_time_rising = 0;
static volatile uint64_t last_time_falling = 0;
static volatile uint64_t last_time_valid_rising = 0;
static volatile uint64_t interval = 0;
static volatile uint32_t last_rpm_value_millis = 0;
static volatile uint8_t rpm_cnt = 0;
static gptimer_handle_t gptimer = 0;
static uint32_t average_intv_q = 0;
static uint32_t rpm_debounce_ticks = DEBOUNCE_TICKS;
static uint32_t rpm_min_low_us = RPM_MIN_LOW_US_DEFAULT;
static uint32_t rpm_min_high_us = RPM_MIN_HIGH_US_DEFAULT;

CircularBuffer rpm_buffer;

// Initialize the circular buffer
void init_buffer(CircularBuffer *buffer) {
    for (int i = 0; i < RPM_WINDOW_SIZE; i++) {
        buffer->data[i] = 0;
    }
    buffer->index = 0;
}

// Insert a new value into the circular buffer
void insert_buffer(CircularBuffer *buffer, int value) {
    buffer->data[buffer->index] = value;
    buffer->index = (buffer->index + 1) % RPM_WINDOW_SIZE;
}

// Function to find the median of the buffer's data
int find_median(CircularBuffer *buffer) {
    int sorted[RPM_WINDOW_SIZE];
    for (int i = 0; i < RPM_WINDOW_SIZE; i++) {
        sorted[i] = buffer->data[i];
    }

    // Sort the window (Insertion sort)
    for (int i = 1; i < RPM_WINDOW_SIZE; i++) {
        int key = sorted[i];
        int j = i - 1;

        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j = j - 1;
        }
        sorted[j + 1] = key;
    }

    // Return the median element
    return sorted[RPM_WINDOW_SIZE / 2];
}

// Median filter function
int median_filter(CircularBuffer *buffer, int new_value) {
    insert_buffer(buffer, new_value);
    return find_median(buffer);
}

static void IRAM_ATTR gpio_isr_handler_stable(void* arg) {
    uint64_t current_time;

    gptimer_get_raw_count(gptimer, &current_time);
    if (gpio_get_level(RPM_PIN) == 1) {
        // Rising
        if ((last_time_falling != 0) && ((current_time - last_time_falling) < rpm_debounce_ticks)) {
            return;
        }

        if (last_time_valid_rising != 0) {
            interval = current_time - last_time_valid_rising;
            uint32_t intv_q = (uint32_t)interval;
            if (intv_q >= rpm_debounce_ticks) {
                xQueueSendFromISR(gpio_evt_queue, &intv_q, NULL);
            }
        }

        last_time_rising = current_time;
        last_time_valid_rising = current_time;
    } else {
        // Falling
        last_time_falling = current_time;
    }
}

static void IRAM_ATTR gpio_isr_handler_legacy(void* arg) {
    uint64_t current_time;

    gptimer_get_raw_count(gptimer, &current_time);
    if (gpio_get_level(RPM_PIN) == 1) {
        if ((last_time_falling != 0) && ((current_time - last_time_falling) < rpm_debounce_ticks)) {
            return;
        }

        if (last_time_valid_rising != 0) {
            interval = current_time - last_time_valid_rising;
            uint32_t intv_q = (uint32_t) interval;
            if (intv_q >= rpm_debounce_ticks) {
                average_intv_q += intv_q;
                rpm_cnt++;
                if ((rpm_cnt & 0x07u) == 0u) {
                    uint32_t avg_intv_q = average_intv_q >> 3;
                    average_intv_q = 0;
                    xQueueSendFromISR(gpio_evt_queue, &avg_intv_q, NULL);
                }
            }
        }

        last_time_rising = current_time;
        last_time_valid_rising = current_time;
    } else {
        // Falling
        last_time_falling = current_time;
    }
}

static void IRAM_ATTR gpio_isr_handler_glitch(void* arg) {
    uint64_t current_time;

    gptimer_get_raw_count(gptimer, &current_time);
    if (gpio_get_level(RPM_PIN) == 1) {
        // Rising
        if ((last_time_falling != 0) && ((current_time - last_time_falling) < rpm_debounce_ticks)) {
            return;
        }

        if (last_time_falling != 0) {
            uint32_t low_us = (uint32_t)(current_time - last_time_falling);
            if (low_us < rpm_min_low_us) {
                return;
            }
        }

        last_time_rising = current_time;
    } else {
        // Falling
        last_time_falling = current_time;

        if (last_time_rising == 0) {
            return;
        }

        uint32_t high_us = (uint32_t)(current_time - last_time_rising);
        if (high_us < rpm_min_high_us) {
            return;
        }

        if (last_time_valid_rising != 0) {
            uint32_t period_us = (uint32_t)(last_time_rising - last_time_valid_rising);
            if (period_us >= rpm_debounce_ticks) {
                xQueueSendFromISR(gpio_evt_queue, &period_us, NULL);
            }
        }

        last_time_valid_rising = last_time_rising;
    }
}

static void tacho_timer_init() {
    ESP_LOGI(TAG, "Create timer handle");
    gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void deinit_gptimer(void)
{
    ESP_LOGI(TAG, "Deinitializing GPTimer...");
    // Stop the GPTimer
    ESP_ERROR_CHECK(gptimer_stop(gptimer));
    // Disable the GPTimer
    ESP_ERROR_CHECK(gptimer_disable(gptimer));
    // Delete the GPTimer
    ESP_ERROR_CHECK(gptimer_del_timer(gptimer));
    gptimer = NULL;
    ESP_LOGI(TAG, "GPTimer deinitialized.");
}

static void frequency_task_stable(void* arg) {
    uint32_t intv = 0;

    while (1) {
        if (xQueueReceive(gpio_evt_queue, &intv, portMAX_DELAY) && intv > 0) {
            uint32_t rpm = 1000000 / intv;
            if (rpm < digifiz_parameters.rpm_sanity_check.value) {
                mRPMSenseData = rpm;
                last_rpm_value_millis = millis();
            }
        }
    }
}

static void frequency_task_legacy(void* arg) {
    uint32_t intv = 0;

    while (1) {
        if (xQueueReceive(gpio_evt_queue, &intv, portMAX_DELAY) && intv > 0) {
            uint32_t rpm_sense = median_filter(&rpm_buffer, 1000000 / intv);
            if (rpm_sense < digifiz_parameters.rpm_sanity_check.value) {
                mRPMSenseData = rpm_sense;
                last_rpm_value_millis = millis();
            }
        }
    }
}

static void frequency_task_glitch(void* arg) {
    uint32_t intv = 0;

    while (1) {
        if (xQueueReceive(gpio_evt_queue, &intv, portMAX_DELAY) && intv > 0) {
            uint32_t rpm = 1000000 / intv;
            if (rpm < digifiz_parameters.rpm_sanity_check.value) {
                mRPMSenseData = rpm;
                last_rpm_value_millis = millis();
            }
        }
    }
}

static void tacho_gpio_init() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << RPM_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    if (digifiz_parameters.rpm_algorithm.value == RPM_ALGORITHM_STABLE) {
        gpio_isr_handler_add(RPM_PIN, gpio_isr_handler_stable, (void*) RPM_PIN);
    } else if (digifiz_parameters.rpm_algorithm.value == RPM_ALGORITHM_GLITCH) {
        gpio_isr_handler_add(RPM_PIN, gpio_isr_handler_glitch, (void*) RPM_PIN);
    } else {
        gpio_isr_handler_add(RPM_PIN, gpio_isr_handler_legacy, (void*) RPM_PIN);
    }
}

void deinit_tacho_gpio(void)
{
    ESP_LOGI(TAG, "Deinitializing GPIO and ISR...");

    // Remove ISR handler
    ESP_ERROR_CHECK(gpio_isr_handler_remove(RPM_PIN));

    // Uninstall ISR service
    gpio_uninstall_isr_service();

    // Reset GPIO configuration
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_DISABLE;
    io_conf.pin_bit_mask = (1ULL << RPM_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(TAG, "GPIO and ISR deinitialized.");
}

void initTacho()
{
    ESP_LOGI(TAG, "initTacho started");
    init_buffer(&rpm_buffer);
    last_rpm_value_millis = millis();
    mRPMSenseData = 0;
    last_time_rising = 0;
    last_time_falling = 0;
    last_time_valid_rising = 0;
    interval = 0;
    average_intv_q = 0;
    rpm_cnt = 0;
    rpm_debounce_ticks = digifiz_parameters.rpm_debounce_ticks.value;
    if (rpm_debounce_ticks == 0) {
        rpm_debounce_ticks = DEBOUNCE_TICKS;
    }
    rpm_min_low_us = digifiz_parameters.rpm_min_low_us.value;
    if (rpm_min_low_us == 0) {
        rpm_min_low_us = RPM_MIN_LOW_US_DEFAULT;
    }
    rpm_min_high_us = digifiz_parameters.rpm_min_high_us.value;
    if (rpm_min_high_us == 0) {
        rpm_min_high_us = RPM_MIN_HIGH_US_DEFAULT;
    }
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    tacho_timer_init();
    tacho_gpio_init();
    // task takes ownership of allocated memory
    if (digifiz_parameters.rpm_algorithm.value == RPM_ALGORITHM_STABLE) {
        xTaskCreatePinnedToCore(frequency_task_stable, "rpm_count_task_stable", 2048, NULL, 10, NULL, 1);
    } else if (digifiz_parameters.rpm_algorithm.value == RPM_ALGORITHM_GLITCH) {
        xTaskCreatePinnedToCore(frequency_task_glitch, "rpm_count_task_glitch", 2048, NULL, 10, NULL, 1);
    } else {
        xTaskCreatePinnedToCore(frequency_task_legacy, "rpm_count_task_legacy", 2048, NULL, 10, NULL, 1);
    }
    ESP_LOGI(TAG, "initTacho ended");
}

uint32_t getRPMDispertion()
{
  return 0;
}

uint32_t getRPMMean()
{
  return 0;
}

//Value in Hz
uint32_t readLastRPM()
{
  if ((millis()-last_rpm_value_millis)<1000)
  {
    return mRPMSenseData;
  }
  else
  {
    return 0;
  }
}
