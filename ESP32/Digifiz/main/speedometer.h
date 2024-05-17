#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

#include <stdint.h>
#include <math.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "driver/pulse_cnt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "frequency_count.h"

#define SPD_M_PCNT_UNIT    PCNT_UNIT_0
#define SPD_M_CHANNEL    PCNT_CHANNEL_0
#define SPD_M_PIN 39

#define SPD_M_SAMPLE_PERIOD 1.0  // seconds
// suitable up to 16,383.5 Hz
#define SPD_M_WINDOW_DURATION 1.0  // seconds
#define SPD_M_FILTER_LENGTH 1023  // APB @ 80MHz, limits to < 39,100 Hz

void initSpeedometer();
uint32_t readLastSpeed();

#endif
