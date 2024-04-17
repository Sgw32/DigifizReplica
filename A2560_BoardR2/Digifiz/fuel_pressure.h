#ifndef FUEL_PRESSURE_H
#define FUEL_PRESSURE_H
#include "Arduino.h"
#include "setup.h"

void initFuelPressureSensor();

void processFuelPressure();
float getFuelPressure();
float getFuelPressureVoltage();

#endif 
