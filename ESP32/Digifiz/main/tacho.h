#ifndef TACHO_H
#define TACHO_H
#include <stdint.h>
#include <math.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define RPM_PIN 35
#define DEBOUNCE_TICKS 50 //0.5ms
#define RPM_WINDOW_SIZE 8  // Adjust as needed for your application

// Circular buffer to hold the most recent data points
typedef struct {
    uint32_t data[RPM_WINDOW_SIZE];
    uint8_t index;
} CircularBuffer;

/**
 * @brief Inits tacho module
 * 
 */
void initTacho();

/**
 * @brief returns filtered RPM in Hz
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

/**
 * @brief resets periph
 */
void deinit_gptimer(void);

/**
 * @brief resets gpio periph
 */
void deinit_tacho_gpio(void);
#endif
