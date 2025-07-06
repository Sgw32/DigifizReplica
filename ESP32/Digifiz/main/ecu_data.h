#ifndef ECU_DATA_H
#define ECU_DATA_H

#include <stdint.h>

typedef struct {
    int8_t engine_temp_c;        // Signed degrees Celsius
    uint8_t throttle_percent;    // %
    uint16_t rpm;                // Revolutions per minute
    uint8_t idle_regulator_pos; // Idle valve %
    float battery_voltage;       // Volts
    float injector_duration_ms;  // ms
    float air_mass_kgph;         // kg/h
    float lambda_voltage;        // Volts
} ecu_data_t;

#endif