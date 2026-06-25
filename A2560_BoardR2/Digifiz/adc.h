#ifndef ADC_H
#define ADC_H

#include "Arduino.h"
#include "ext_eeprom.h"

/**
 * @brief Initializes ADC channels, filters, and persisted calibration parameters.
 */
void initADC();

/**
 * @brief Reloads ADC conversion settings from persistent parameters.
 */
void updateADCSettings();
/// @name Raw ADC helper accessors (0..1024)
/// @{
uint16_t getRawCoolantTemperature();
uint16_t getRawOilTemperature();
uint16_t getRawGasLevel();
uint16_t getRawAmbientTemperature();
uint16_t getRawLightLevel();
/// @}

/// @name Display-oriented helper values
/// @{
uint8_t getLitresInTank(); //0..99
uint8_t getGallonsInTank(); //0..99
uint8_t getDisplayedCoolantTemp(); //0..14
uint8_t getDisplayedOilTemp(); //0..16
uint8_t getDisplayedCoolantTempOrig(); //0..20
/// @}


/// @name Physical values converted from sensors
/// @{
float getCoolantTemperature(); //celsius
float getOilTemperature(); //celsius
float getGasLevel(float R); //percents
float getRToFuelLevel();
float getAmbientTemperature(); //celsius
float getFuelConsumption();

float getIntakePressure();
float getCurrentIntakeFuelConsumption();
float getIntakeVoltage();

float getOilTemperatureFahrenheit(); //F
float getAmbientTemperatureFahrenheit(); //F

uint8_t getBrightnessLevel();
uint16_t getRawBrightnessLevel();
/// @}

/**
 * @brief Updates all sensor filters with new samples.
 */
void processCoolantTemperature();
void processOilTemperature();
void processGasLevel();
void processAmbientTemperature();
void processBrightnessLevel();

/**
 * @brief Seeds filtering state on startup to avoid zero-biased values.
 */
void processFirstCoolantTemperature(); //to prevent filtering from zero value
void processFirstOilTemperature();
void processFirstGasLevel();
void processFirstAmbientTemperature();

#endif
