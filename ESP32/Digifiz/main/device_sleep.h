// device_sleep.h

#ifndef DEVICE_SLEEP_H
#define DEVICE_SLEEP_H

#include "esp_err.h"
#include "params.h"
#include <stdint.h>

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
 * @brief Save the current time to the power backup.
 */
void save_power_time(void);

/**
 * @brief Checks and invalidates time backup if the time since last save exceeds POWER_TIME_VALID_MS.
 * 
 * @return ESP_OK on success
 */
esp_err_t check_invalidate_power_time(void);

/**
 * @brief Read the time stored by save_power_time().
 *
 * @param seconds Receives Unix time in seconds.
 * @param useconds Receives the microsecond part of the saved time.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG for null output pointers, or
 *         the NVS error returned while reading the backup.
 */
esp_err_t device_get_power_backup_time(int64_t *seconds, int32_t *useconds);

/**
 * @brief Read the invalidated time stored by save_power_time().
 *
 * @param seconds Receives Unix time in seconds.
 * @param useconds Receives the microsecond part of the saved time.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG for null output pointers, or
 *         the NVS error returned while reading the backup.
 */
esp_err_t device_get_inv_backup_time(int64_t *seconds, int32_t *useconds);



/**
 * @brief gets the fact off time restoration
 * 
 *
 */
bool get_time_is_restored();

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
