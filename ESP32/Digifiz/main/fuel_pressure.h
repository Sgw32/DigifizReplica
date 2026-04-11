#ifndef FUEL_PRESSURE_H
#define FUEL_PRESSURE_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file fuel_pressure.h
 * @brief Fuel pressure sensor initialization interface.
 */

#include "setup.h"
#include <stdint.h>
#include "adc.h"

/**
 * @brief Initialize fuel pressure sensing subsystem and defaults.
 */
void initFuelPressureSensor();

#ifdef __cplusplus
}
#endif
#endif 
