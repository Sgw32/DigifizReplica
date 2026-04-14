#ifndef TACHO_H
#define TACHO_H
/**
 * @file tacho.h
 * @brief Engine RPM pulse capture and filtering interface.
 */

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

#ifdef __cplusplus
extern "C" {
#endif

/** @brief GPIO connected to the tachometer pulse source. */
#define RPM_PIN 35
/** @brief Debounce threshold in timer ticks for pulse qualification. */
#define DEBOUNCE_TICKS 50 //0.5ms
/** @brief Moving window size used by RPM filter internals. */
#define RPM_WINDOW_SIZE 8  // Adjust as needed for your application

/** @brief Circular buffer for recent RPM samples. */
typedef struct {
    /** @brief Sample storage array (latest values in ring order). */
    uint32_t data[RPM_WINDOW_SIZE];
    /** @brief Write index in @ref data ring buffer. */
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

#ifdef __cplusplus
}
#endif
#endif
