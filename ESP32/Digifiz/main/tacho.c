#include "tacho.h"
#include "setup.h"
#include "esp_log.h"

uint32_t mRPMSenseData;

#define TAG "tacho"
static QueueHandle_t gpio_evt_queue = NULL;
static volatile uint64_t last_time_rising = 0;
static volatile uint64_t last_time_falling = 0;
static volatile uint64_t interval = 0;
static volatile uint8_t rpm_cnt = 0;
static gptimer_handle_t gptimer = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint64_t current_time;
    gptimer_get_raw_count(gptimer, &current_time);
    uint32_t current_gpio_level = gpio_get_level(RPM_PIN);
    if (current_gpio_level==1)
    {
      //Rising
      if ((last_time_falling-current_time)>DEBOUNCE_TICKS)
      {
        interval = current_time - last_time_rising;
      }
      last_time_rising = current_time;
      uint32_t intv_q = (uint32_t) interval;
      rpm_cnt++;
      if (rpm_cnt%10)
      {
        xQueueSendFromISR(gpio_evt_queue, &intv_q, NULL);
      }
    }
    else
    {
      //Falling
      last_time_falling = current_time;
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

static void frequency_task(void* arg) {
    uint64_t intv;
    while (1) {
       if (xQueueReceive(gpio_evt_queue, &intv, portMAX_DELAY)) {
            mRPMSenseData = 1000000/intv;
       }
      // uint64_t current_time;
      // gptimer_get_raw_count(gptimer, &current_time);
      // interval = current_time - last_time;
      // last_time = current_time;
      // printf("interval: %llu\n", interval);
      // vTaskDelay(100 / portTICK_PERIOD_MS);
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

    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    gpio_isr_handler_add(RPM_PIN, gpio_isr_handler, (void*) RPM_PIN);
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
    mRPMSenseData = 0;
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    tacho_timer_init();
    tacho_gpio_init();
    // task takes ownership of allocated memory
    xTaskCreatePinnedToCore(frequency_task, "rpm_count_task", 2048, NULL, 10, NULL, 1);
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

uint32_t readLastRPM()
{
  return mRPMSenseData;
}
