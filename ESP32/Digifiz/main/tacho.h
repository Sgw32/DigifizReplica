#ifndef TACHO_H
#define TACHO_H
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

#define RPM_PCNT_UNIT    PCNT_UNIT_0
#define RPM_CHANNEL    PCNT_CHANNEL_1
#define RPM_PIN 35

#define RPM_SAMPLE_PERIOD 0.05  // seconds
// suitable up to 16,383.5 Hz
#define RPM_WINDOW_DURATION 0.05  // seconds
/**
 * @brief Inits tacho module
 * 
 */
void initTacho();

/**
 * @brief returns filtered RPM in micros
 * 
 * @return uint32_t 
 */
uint32_t readLastRPM();

/**
 * @brief Reads the dispertion of RPM which helps to filter out unusable results
 * 
 * @return uint32_t 
 */
uint32_t getRPMDispertion();

/**
 * @brief gets RPM mean from the filter
 * 
 * @return uint32_t 
 */
uint32_t getRPMMean();
#endif
