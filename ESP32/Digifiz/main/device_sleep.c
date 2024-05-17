// device_sleep.c
#include "device_sleep.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdio.h>

#define SLEEP_PIN GPIO_NUM_10

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
    return ESP_OK;
}

bool device_sleep_check() {
    return gpio_get_level(SLEEP_PIN) == 1;
}

void device_sleep_dump() {
    printf("Sleep pin state: %s\n", device_sleep_check() ? "Sleep" : "Not sleep");
}
