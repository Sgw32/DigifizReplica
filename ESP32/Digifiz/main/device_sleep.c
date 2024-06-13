// device_sleep.c
#include "device_sleep.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdio.h>

#define SLEEP_PIN GPIO_NUM_10
#define POWER_OUT_PIN GPIO_NUM_47


esp_err_t initDeviceSleep() {
    ESP_LOGI(LOG_TAG, "initDeviceSleep started");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << SLEEP_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    ESP_LOGI(LOG_TAG, "initDeviceSleep ended");

    gpio_config_t pout_cfg_out = {
        .pin_bit_mask = BIT64(POWER_OUT_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&pout_cfg_out);
    return ESP_OK;
}

bool device_sleep_check() {
    //TODO tests...
    bool sleepMode = gpio_get_level(SLEEP_PIN) == 1;
    if (sleepMode)
    {
        gpio_set_level(POWER_OUT_PIN, 0);
    }
    else
    {
        gpio_set_level(POWER_OUT_PIN, 1);
    }
    return sleepMode;
}

void device_sleep_dump() {
    printf("Sleep pin state: %s\n", device_sleep_check() ? "Sleep" : "Not sleep");
}
