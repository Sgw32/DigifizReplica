#include "tacho.h"
#include "setup.h"
#include "esp_log.h"

uint32_t mRPMSenseData;

#define TAG "tacho"

pcnt_unit_handle_t rpm_pcnt_unit;
//TODO mutex
static void tacho_callback(double hz)
{
    mRPMSenseData = (uint32_t)fabs(hz);
    // ESP_LOGI(TAG, "Tacho: %f Hz", hz);
}

void initTacho()
{
    ESP_LOGI(TAG, "initTacho started");
    mRPMSenseData = 0;
    frequency_count_configuration_t * config = malloc(sizeof(*config));
    config->pcnt_gpio = RPM_PIN;
    config->pcnt_unit = &rpm_pcnt_unit;
    config->sampling_period_seconds = RPM_SAMPLE_PERIOD;
    config->sampling_window_seconds = RPM_WINDOW_DURATION;
    config->frequency_update_callback = &tacho_callback;

    // task takes ownership of allocated memory
    xTaskCreatePinnedToCore(&frequency_count_task_function, "rpm_count_task", 4096, config, 5, NULL, 1);
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
