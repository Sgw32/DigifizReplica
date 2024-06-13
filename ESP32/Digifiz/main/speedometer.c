#include "speedometer.h"
#include "setup.h"
#include "esp_log.h"

uint32_t mSpdData;

#define TAG "spd_m"
pcnt_unit_handle_t spd_m_pcnt_unit;
//TODO mutex
static void spd_m_callback(double hz)
{
    mSpdData = (uint32_t)fabs(hz);
    ESP_LOGI(TAG, "SPD_M: %f Hz", hz);
}

void initSpeedometer()
{
    ESP_LOGI(TAG, "initSpeedometer started");
    frequency_count_configuration_t * config = malloc(sizeof(*config));
    config->pcnt_gpio = SPD_M_PIN;
    config->pcnt_unit = &spd_m_pcnt_unit;
    config->sampling_period_seconds = SPD_M_SAMPLE_PERIOD;
    config->sampling_window_seconds = SPD_M_WINDOW_DURATION;
    config->frequency_update_callback = &spd_m_callback;

    // task takes ownership of allocated memory
    xTaskCreatePinnedToCore(&frequency_count_task_function, "spd_m_count_task", 4096, config, 6, NULL,1);
    ESP_LOGI(TAG, "initSpeedometer ended");
}

uint32_t readLastSpeed()
{
   return mSpdData;
}
