/*
 * MIT License
 *
 * Copyright (c) 2018 Chris Morgan <chmorgan@gmail.com>
 * Copyright (c) 2018 David Antliff
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

/**
 * @file frequency_count.h
 * @brief Continuous ESP32 pulse counting with a rolling measurement window.
 *
 * PCNT counts falling edges continuously, with hardware glitch filtering and
 * overflow accumulation. Cumulative count snapshots and their actual elapsed
 * timestamps determine frequency; counts are not cleared between readings.
 * It should not be used for any external purposes.
 *
 * Usage:
 * 
 * - Create an instance of frequency_count_configuration_t and set its values appropriately
 * - Create a task with frequency_count_task() as its function, passing a pointer
 *   to the frequency_count_configuration_t into the task creation function
 * 
 * 
 */

#pragma once
#ifndef FREQUENCY_COUNT_H
#define FREQUENCY_COUNT_H

#include <stdint.h>
#include "driver/pulse_cnt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    pcnt_unit_handle_t* pcnt_unit;
    gpio_num_t pcnt_gpio;                   ///< count events on this GPIO
    float sampling_period_seconds;          ///< time (in seconds) between start of adjacent samples
    float sampling_window_seconds;          ///< sample window length (in seconds)
    uint16_t filter_length;                 ///< glitch filter in 80 MHz APB cycles (0: 1 us default)
    void (*frequency_update_callback)(double hz);  ///< called each time a frequency is determined
} frequency_count_configuration_t;

/**
 * @brief Task function - pass this to xTaskCreate with a pointer to an instance of frequency_count_configuration_t as the parameter.
 */
void frequency_count_task_function(void * pvParameter);

#ifdef __cplusplus
}
#endif

#endif // FREQUENCY_COUNT
