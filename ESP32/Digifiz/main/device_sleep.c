// device_sleep.c
#include "device_sleep.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "setup.h"
#include "display_next.h"
#include "digifiz_watchdog.h"
#include "mfa.h"
#include "nvs.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include "millis.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Include semaphore/mutex header
#include "driver/rtc_io.h"


// Power on
// Check if we need to restore the time immediately after the boot
// Save time to backup right after boot 
// Turn on the power but keep the brightness zero
// After 300 ms enable brightness and invalidate backup

#define SLEEP_PIN GPIO_NUM_10
#define POWER_OUT_PIN GPIO_NUM_47
#define POWER_TIME_NAMESPACE "power_backup"
#define POWER_TIME_SECONDS_KEY "time_seconds"
#define POWER_TIME_USECONDS_KEY "time_useconds"
#define POWER_TIME_VALID_MS 3000


const int ext_wakeup_pin_1 = SLEEP_PIN;
const uint64_t ext_wakeup_pin_1_mask = 1ULL << SLEEP_PIN;
static volatile uint32_t power_enable_generation;
static uint8_t time_is_restored = 0;
uint32_t save_millis = 0;
int64_t backup_seconds = 0;
int32_t backup_useconds = 0;

uint8_t backup_time_invalidated = 0;

esp_err_t check_invalidate_power_time(void)
{
    esp_err_t err = ESP_OK;
    if (!backup_time_invalidated)
    {
        nvs_handle_t handle;
        if (millis() > (save_millis + POWER_TIME_VALID_MS)) {
            device_get_power_backup_time(&backup_seconds, &backup_useconds);
            esp_err_t err = nvs_open(POWER_TIME_NAMESPACE, NVS_READWRITE, &handle);
            if (err == ESP_OK) {
                err = nvs_set_i64(handle, POWER_TIME_SECONDS_KEY, 0);
                if (err == ESP_OK) {
                    err = nvs_set_i32(handle, POWER_TIME_USECONDS_KEY, 0);
                }
                if (err == ESP_OK) {
                    err = nvs_commit(handle);
                }
                nvs_close(handle);
            }
            backup_time_invalidated = true;
        }
    }
    return err;
}

// static void invalidate_power_time_task(void *args)
// {
//     const uint32_t generation = (uint32_t)(uintptr_t)args;
//     vTaskDelay(pdMS_TO_TICKS(POWER_TIME_VALID_MS));

//     if (generation == power_enable_generation && gpio_get_level(POWER_OUT_PIN) == 1) {
//         esp_err_t err = invalidate_power_time();
//         if (err == ESP_OK) {
//             ESP_LOGI(LOG_TAG, "POWER_OUT remained enabled; power time backup invalidated");
//         } else {
//             ESP_LOGE(LOG_TAG, "Failed to invalidate power time backup: %s", esp_err_to_name(err));
//         }
//     }
//     vTaskDelete(NULL);
// }

void save_power_time(void)
{
    struct timeval now;
    nvs_handle_t handle;
    save_millis = millis();
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

esp_err_t device_get_inv_backup_time(int64_t *seconds, int32_t *useconds)
{
    if (seconds == NULL || useconds == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *seconds = backup_seconds;
    *useconds = backup_useconds;
    return ESP_OK;
}

esp_err_t device_get_power_backup_time(int64_t *seconds, int32_t *useconds)
{
    if (seconds == NULL || useconds == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *seconds = 0;
    *useconds = 0;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(POWER_TIME_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_i64(handle, POWER_TIME_SECONDS_KEY, seconds);
    if (err == ESP_OK) {
        err = nvs_get_i32(handle, POWER_TIME_USECONDS_KEY, useconds);
    }
    nvs_close(handle);

    return err;
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
        time_is_restored = 1;
        if (settimeofday(&restored, NULL) == 0) {
            ESP_LOGI(LOG_TAG, "Restored time saved before POWER_OUT reset");
            nvs_close(handle);
            return;
        } else {
            ESP_LOGE(LOG_TAG, "Failed to restore time saved before POWER_OUT reset");
        }
    }
    nvs_close(handle);
}

bool get_time_is_restored()
{
    return time_is_restored;
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
        resetBrightness();
        device_power_enable(false);
    }
    else
    {        
        device_power_enable(true);
    }
    return sleepMode;
}

static uint8_t prev_enable = 0;

void device_power_enable(bool enable)
{
    if (enable)
    {
        if (prev_enable==0) 
        {
            startMFAOperationPause();
            resetBrightness();
            gpio_set_level(POWER_OUT_PIN, 1);
            prev_enable = enable;
            //const uint32_t generation = ++power_enable_generation;
            // if (xTaskCreate(invalidate_power_time_task, "power_time_clear", 3072,
            //                 (void *)(uintptr_t)generation, 2, NULL) != pdPASS) {
            //     ESP_LOGE(LOG_TAG, "Failed to schedule power time backup invalidation");
            // }
            return;
        }
        gpio_set_level(POWER_OUT_PIN, 1);
    }
    else
    {
        //++power_enable_generation;
        startMFAOperationPause();
        gpio_set_level(POWER_OUT_PIN, 0);
    }
    prev_enable = enable;
}

void device_sleep_dump() {
    printf("Sleep pin state: %s\n", device_sleep_check() ? "Sleep" : "Not sleep");
}
