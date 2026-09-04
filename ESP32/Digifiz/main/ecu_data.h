#ifndef ECU_DATA_H
#define ECU_DATA_H

/**
 * @file ecu_data.h
 * @brief Normalized ECU telemetry data container.
 */

#include <stdint.h>

/**
 * @brief Snapshot of decoded ECU telemetry values.
 */
typedef struct {
    /** @brief Engine coolant temperature in degrees Celsius. */
    int8_t engine_temp_c;        // Signed degrees Celsius
    /** @brief Throttle opening in percent (0..100). */
    uint8_t throttle_percent;    // %
    /** @brief Engine speed in revolutions per minute. */
    uint16_t rpm;                // Revolutions per minute
    /** @brief Idle regulator/valve position in percent. */
    uint8_t idle_regulator_pos; // Idle valve %
    /** @brief Electrical system voltage in volts. */
    float battery_voltage;       // Volts
    /** @brief Injector pulse duration in milliseconds. */
    float injector_duration_ms;  // ms
    /** @brief Air mass flow in kilograms per hour. */
    float air_mass_kgph;         // kg/h
    /** @brief Lambda probe voltage in volts. */
    float lambda_voltage;        // Volts
} ecu_data_t;

#endif
