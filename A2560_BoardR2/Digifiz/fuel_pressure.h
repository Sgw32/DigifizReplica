#ifndef FUEL_PRESSURE_H
#define FUEL_PRESSURE_H
/**
 * @file fuel_pressure.h
 * @brief Fuel pressure sensing interface for the A2560 board firmware.
 */

#include "Arduino.h"
#include "setup.h"

/**
 * @brief Initializes ADC/pin resources used by the fuel pressure sensor.
 */
void initFuelPressureSensor();

/**
 * @brief Samples and filters the fuel pressure signal.
 */
void processFuelPressure();

/**
 * @brief Returns the latest computed fuel pressure in bar.
 *
 * @return Fuel pressure value.
 */
float getFuelPressure();

/**
 * @brief Returns the latest measured sensor voltage.
 *
 * @return Fuel pressure sensor voltage.
 */
float getFuelPressureVoltage();

#endif 
