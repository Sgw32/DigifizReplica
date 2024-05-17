/*
 * MIT License
 *
 * Copyright (c) 2018 David Antliff
 * Copyright (c) 2018 Chris Morgan <chmorgan@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

#ifdef INCLUDE_ESP_IDF_VERSION_H
# include "esp_idf_version.h"
#endif

#include "frequency_count.h"

#define TAG "frequency_counter"

static void init_pcnt(uint8_t pulse_gpio, pcnt_unit_handle_t* pcnt_unit)
{
    ESP_LOGD(TAG, "%s", __FUNCTION__);

    // set up counter
    pcnt_unit_config_t unit_config = {
    .high_limit = 4000,
    .low_limit = -4000,
    };
    
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, pcnt_unit));

    ESP_LOGI(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(*pcnt_unit, &filter_config));

    pcnt_chan_config_t chan_config = {
    .edge_gpio_num = pulse_gpio,
    .level_gpio_num = -1,
    };
    pcnt_channel_handle_t pcnt_chan = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(*pcnt_unit, &chan_config, &pcnt_chan));

    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE));

    ESP_LOGI(TAG, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(*pcnt_unit));
    ESP_LOGI(TAG, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(*pcnt_unit));
    ESP_LOGI(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(*pcnt_unit));
}

void frequency_count_task_function(void * pvParameter)
{
    frequency_count_configuration_t configuration;

    assert(pvParameter);
    ESP_LOGI(TAG, "Core ID %d", xPortGetCoreID());

    configuration = *(frequency_count_configuration_t*)pvParameter;
    frequency_count_configuration_t *task_inputs = &configuration;

    ESP_LOGI(TAG, "pcnt_gpio %d, sampling_period_seconds %f, sampling_window_seconds %f" ,
        task_inputs->pcnt_gpio,
        task_inputs->sampling_period_seconds,
        task_inputs->sampling_window_seconds);

    init_pcnt(task_inputs->pcnt_gpio,task_inputs->pcnt_unit);

    TickType_t last_wake_time = xTaskGetTickCount();
    double frequency_hz;

    while (1)
    {
        // read counter
        int pulse_count = 0;
        ESP_ERROR_CHECK(pcnt_unit_get_count(*task_inputs->pcnt_unit, &pulse_count));
        // clear counter
        ESP_ERROR_CHECK(pcnt_unit_clear_count(*task_inputs->pcnt_unit));

        // TODO: check for overflow?

        frequency_hz = pulse_count / task_inputs->sampling_window_seconds;

        // call the frequency update callback
        if (task_inputs->frequency_update_callback)
        {
            (task_inputs->frequency_update_callback)(frequency_hz);
        }

        // ESP_LOGI(TAG, "counter %d, frequency %f Hz", pulse_count, frequency_hz);

        int delay_time = task_inputs->sampling_period_seconds * 1000 / portTICK_PERIOD_MS;
        if (delay_time > 0)
        {
            vTaskDelayUntil(&last_wake_time, delay_time);
        }
    }
    free(task_inputs);  // TODO: avoid this if not dynamically allocated
    vTaskDelete(NULL);
}
