#ifndef ADC_H
#define ADC_H

#include "Arduino.h"
#include "eeprom.h"

void initADC();

//RAW values 0..1024
uint16_t getRawCoolantTemperature();
uint16_t getRawOilTemperature();
uint16_t getRawGasLevel();
uint16_t getRawAmbientTemperature();
uint16_t getRawLightLevel();

//Display helpers
uint8_t getLitresInTank(); //0..99
uint8_t getDisplayedCoolantTemp(); //0..14


//Physical data values
float getCoolantTemperature(); //celsius
float getOilTemperature(); //celsius
float getGasLevel(); //percents
float getAmbientTemperature(); //celsius

void processCoolantTemperature();
void processOilTemperature();
void processGasLevel();
void processAmbientTemperature();

void processFirstCoolantTemperature(); //to prevent filtering from zero value
void processFirstOilTemperature();
void processFirstGasLevel();
void processFirstAmbientTemperature();

#endif
