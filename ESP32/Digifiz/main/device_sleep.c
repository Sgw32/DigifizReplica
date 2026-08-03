// device_sleep.c
#include "device_sleep.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "setup.h"
#include "display_next.h"
#include "digifiz_watchdog.h"
#include "nvs.h"
#include <stdio.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Include semaphore/mutex header
#include "driver/rtc_io.h"

#define SLEEP_PIN GPIO_NUM_10
#define POWER_OUT_PIN GPIO_NUM_47
#define POWER_TIME_NAMESPACE "power_backup"
#define POWER_TIME_SECONDS_KEY "time_seconds"
#define POWER_TIME_USECONDS_KEY "time_useconds"


const int ext_wakeup_pin_1 = SLEEP_PIN;
const uint64_t ext_wakeup_pin_1_mask = 1ULL << SLEEP_PIN;

static void save_power_time(void)
{
    struct timeval now;
    nvs_handle_t handle;

    gettimeofday(&now, NULL);
    esp_err_t err = nvs_open(POWER_TIME_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        err = nvs_set_i64(handle, POWER_TIME_SECONDS_KEY, (int64_t)now.tv_sec);
        if (err == ESP_OK) {
            err = nvs_set_i32(handle, POWER_TIME_USECONDS_KEY, (int32_t)now.tv_usec);
        }
        if (err == ESP_OK) {
            err = nvs_commit(handle);
        }
        nvs_close(handle);
    }

    if (err != ESP_OK) {
        ESP_LOGE(LOG_TAG, "Failed to back up time before power enable: %s", esp_err_to_name(err));
    }
}

void device_restore_power_time(void)
{
    nvs_handle_t handle;
    int64_t seconds = 0;
    int32_t useconds = 0;
    esp_err_t err = nvs_open(POWER_TIME_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(LOG_TAG, "Could not open power time backup: %s", esp_err_to_name(err));
        return;
    }

    esp_err_t seconds_err = nvs_get_i64(handle, POWER_TIME_SECONDS_KEY, &seconds);
    esp_err_t useconds_err = nvs_get_i32(handle, POWER_TIME_USECONDS_KEY, &useconds);
    if (seconds_err == ESP_OK && useconds_err == ESP_OK && seconds != 0) {
        const struct timeval restored = {
            .tv_sec = (time_t)seconds,
            .tv_usec = (suseconds_t)useconds,
        };
        if (settimeofday(&restored, NULL) == 0) {
            ESP_LOGI(LOG_TAG, "Restored time saved before POWER_OUT reset");
            nvs_set_i64(handle, POWER_TIME_SECONDS_KEY, 0);
            nvs_set_i32(handle, POWER_TIME_USECONDS_KEY, 0);
            err = nvs_commit(handle);
            if (err != ESP_OK) {
                ESP_LOGE(LOG_TAG, "Failed to clear restored power time: %s", esp_err_to_name(err));
            }
        } else {
            ESP_LOGE(LOG_TAG, "Failed to restore time saved before POWER_OUT reset");
        }
    }
    nvs_close(handle);
}

static void deep_sleep_task(void *args)
{
    printf("Wake up on\n");
    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER: {
            printf("Wake up from timer.");
            break;
        }
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            if (wakeup_pin_mask != 0) {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                printf("Wake up from GPIO %d\n", pin);
            } else {
                printf("Wake up from GPIO\n");
            }
            // Reinitialize the watchdog timer after waking from deep sleep
            if (watchdog_reinit() != ESP_OK) {
                ESP_LOGE("Watchdog", "Failed to reinitialize watchdog timer");
            }
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset\n");
    }

    rtc_gpio_deinit(SLEEP_PIN);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << SLEEP_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
    while (1)
    {
        //printf("Checking sleep pin...\n");
#ifndef DEBUG_SLEEP_DISABLE
        bool sleepMode = gpio_get_level(SLEEP_PIN) == 1;
        if (sleepMode)
        {
            //printf("Entering deep sleep\n");
            watchdog_delete();
            ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ANY_LOW));
            ESP_ERROR_CHECK(rtc_gpio_pulldown_dis(ext_wakeup_pin_1));
            ESP_ERROR_CHECK(rtc_gpio_pullup_en(ext_wakeup_pin_1));
            esp_deep_sleep_start();
        }
#endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// static void example_deep_sleep_register_rtc_timer_wakeup(void)
// {
//     const int wakeup_time_sec = 20;
//     printf("Enabling timer wakeup, %ds\n", wakeup_time_sec);
//     ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
// }

esp_err_t initDeviceSleep() {
    ESP_LOGI(LOG_TAG, "initDeviceSleep started");
    //example_deep_sleep_register_rtc_timer_wakeup();

    gpio_config_t pout_cfg_out = {
        .pin_bit_mask = BIT64(POWER_OUT_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&pout_cfg_out);

    xTaskCreate(deep_sleep_task, "deep_sleep_task", 4096, NULL, 6, NULL);
    ESP_LOGI(LOG_TAG, "initDeviceSleep ended");
    return ESP_OK;
}

bool device_sleep_check() {
    //TODO tests...
    bool sleepMode = gpio_get_level(SLEEP_PIN) == 1;
    if (sleepMode)
    {
#ifndef DEBUG_SLEEP_DISABLE
        resetBrightness();
#endif
        gpio_set_level(POWER_OUT_PIN, 0);
    }
    else
    {
        device_power_enable(true);
    }
    return sleepMode;
}

void device_power_enable(bool enable)
{
    if (enable)
    {
        if (gpio_get_level(POWER_OUT_PIN) == 0) {
            save_power_time();
        }
        gpio_set_level(POWER_OUT_PIN, 1);
    }
    else
    {
        gpio_set_level(POWER_OUT_PIN, 0);
    }
}

void device_sleep_dump() {
    printf("Sleep pin state: %s\n", device_sleep_check() ? "Sleep" : "Not sleep");
}
