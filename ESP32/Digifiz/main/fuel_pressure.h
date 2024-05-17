#ifndef FUEL_PRESSURE_H
#define FUEL_PRESSURE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "setup.h"
#include <stdint.h>


void initFuelPressureSensor();

void processFuelPressure();
float getFuelPressure();
float getFuelPressureVoltage();

#ifdef __cplusplus
}
#endif
#endif 
