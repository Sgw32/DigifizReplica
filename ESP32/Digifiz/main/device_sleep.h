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

/**
 * @brief Enable or disable device power. For 
 * 
 * @param enable 
 */
void device_power_enable(bool enable);

/**
 * @brief Restore a time saved immediately before POWER_OUT was enabled.
 *
 * This must be called after NVS initialization and before enabling POWER_OUT.
 * A successfully consumed backup is cleared so it cannot be reused.
 */
void device_restore_power_time(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_SLEEP_H */
