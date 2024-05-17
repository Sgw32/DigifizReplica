// device_sleep.h

#ifndef DEVICE_SLEEP_H
#define DEVICE_SLEEP_H

#include "esp_err.h"
#include "params.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize GPIO pin for sleep monitoring.
 *
 * @return
 *     - ESP_OK on success
 *     - Error code otherwise
 */
esp_err_t initDeviceSleep();

/**
 * @brief Check the state of the sleep pin.
 *
 * @return true if device is in sleep mode, false otherwise.
 */
bool device_sleep_check();

/**
 * @brief Print the state of the sleep pin.
 */
void device_sleep_dump();

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_SLEEP_H */
