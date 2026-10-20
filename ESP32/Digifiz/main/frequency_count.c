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
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "esp_timer.h"
#include "frequency_window.h"

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

static void init_pcnt(uint8_t pulse_gpio, pcnt_unit_handle_t* pcnt_unit,
                      uint16_t filter_length)
{
    ESP_LOGD(TAG, "%s", __FUNCTION__);

    // set up counter
    pcnt_unit_config_t unit_config = {
    .high_limit = 4000,
    .low_limit = -4000,
    .flags.accum_count = true,
    };
    
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, pcnt_unit));

    ESP_LOGI(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        // Existing configuration is in APB cycles, at the ESP32-S3's 80 MHz.
        .max_glitch_ns = filter_length ? ((uint32_t)filter_length * 1000u + 79u) / 80u : 1000u,
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
    ESP_ERROR_CHECK(gpio_pullup_en(pulse_gpio));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(*pcnt_unit, 4000));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(*pcnt_unit, -4000));

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
    free(pvParameter);
    frequency_count_configuration_t *task_inputs = &configuration;

    ESP_LOGI(TAG, "pcnt_gpio %d, sampling_period_seconds %f, sampling_window_seconds %f" ,
        task_inputs->pcnt_gpio,
        task_inputs->sampling_period_seconds,
        task_inputs->sampling_window_seconds);

    init_pcnt(task_inputs->pcnt_gpio, task_inputs->pcnt_unit, task_inputs->filter_length);

    // Keep a cumulative history for the actual configured window. The old
    // read/clear pair lost pulses between those calls and divided a period's
    // count by a potentially different window duration.
    assert(isfinite(task_inputs->sampling_period_seconds) && task_inputs->sampling_period_seconds > 0);
    assert(isfinite(task_inputs->sampling_window_seconds) && task_inputs->sampling_window_seconds > 0);
    uint32_t period_ms = (uint32_t)(task_inputs->sampling_period_seconds * 1000.0f);
    TickType_t delay_ticks = pdMS_TO_TICKS(period_ms);
    if (delay_ticks == 0) {
        delay_ticks = 1;
    }
    uint64_t period_us = (uint64_t)delay_ticks * portTICK_PERIOD_MS * 1000u;
    uint64_t window_us = (uint64_t)(task_inputs->sampling_window_seconds * 1000000.0f);
    unsigned capacity = (unsigned)((window_us + period_us - 1u) / period_us) + 2u;
    frequency_snapshot_t *samples = calloc(capacity, sizeof(*samples));
    assert(samples != NULL);
    frequency_window_t window = {
        .samples = samples, .capacity = capacity, .window_us = window_us,
    };
    int initial_count;
    ESP_ERROR_CHECK(pcnt_unit_get_count(*task_inputs->pcnt_unit, &initial_count));
    frequency_window_push(&window, (uint64_t)esp_timer_get_time(), (uint32_t)initial_count);

    TickType_t last_wake_time = xTaskGetTickCount();
    double frequency_hz;

    while (1)
    {
        vTaskDelayUntil(&last_wake_time, delay_ticks);
        // read counter
        int pulse_count = 0;
        ESP_ERROR_CHECK(pcnt_unit_get_count(*task_inputs->pcnt_unit, &pulse_count));
        frequency_hz = frequency_window_push(&window, (uint64_t)esp_timer_get_time(),
                                             (uint32_t)pulse_count);

        // call the frequency update callback
        if (task_inputs->frequency_update_callback)
        {
            (task_inputs->frequency_update_callback)(frequency_hz);
        }

        // ESP_LOGI(TAG, "counter %d, frequency %f Hz", pulse_count, frequency_hz);

    }
    free(samples);
    vTaskDelete(NULL);
}
