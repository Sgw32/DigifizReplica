#ifndef ADC_H
#define ADC_H

#include "Arduino.h"

void initADC();

//RAW values 0..1024
uint16_t getRawCoolantTemperature();
uint16_t getRawOilTemperature();
uint16_t getRawGasLevel();
uint16_t getRawAmbientTemperature();
uint16_t getRawLightLevel();

//Data values
float getCoolantTemperature();
float getOilTemperature();
float getGasLevel();
float getAmbientTemperature();

#endif