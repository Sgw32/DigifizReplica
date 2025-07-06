#ifndef BLE_MODULE_H
#define BLE_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Initialize the BLE module
 * 
 * Sets up the BLE stack, registers the GATT service and characteristics,
 * and starts advertising. Automatically resumes advertising after disconnect.
 */
void ble_module_init(void);

/**
 * @brief Get the current value of characteristic 1 (CHAR1_UUID)
 * 
 * @return uint32_t Current value
 */
uint32_t ble_module_get_char1_value(void);

/**
 * @brief Get the current value of characteristic 2 (CHAR2_UUID)
 * 
 * @return uint32_t Current value
 */
uint32_t ble_module_get_char2_value(void);

/**
 * @brief Set the value of characteristic 1 (CHAR1_UUID)
 * 
 * @param value New value to set
 */
void ble_module_set_char1_value(uint32_t value);

/**
 * @brief Set the value of characteristic 2 (CHAR2_UUID)
 * 
 * @param value New value to set
 */
void ble_module_set_char2_value(uint32_t value);

/**
 * @brief Update the values of RPM and Speed characteristics
 * 
 * This function updates the BLE characteristics with the latest RPM and Speed values.
 * It should be called periodically to keep the BLE data fresh.
 * 
 * @param rpm Current RPM value
 * @param speed Current Speed value in KMH/MPH
 */
void ble_module_update_values(float rpm, float speed);

#ifdef __cplusplus
}
#endif

#endif // BLE_MODULE_H
