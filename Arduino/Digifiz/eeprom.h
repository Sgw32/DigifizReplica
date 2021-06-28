#ifndef EEPROM_H
#define EEPROM_H

#include <Wire.h>
#include <SparkFun_External_EEPROM.h>
#include "Arduino.h"

typedef struct digifiz_pars
{
    uint8_t rpmCoefficient; //used
    uint8_t speedCoefficient; //used
    int coolantThermistorB; //used
    int oilThermistorB; 
    int airThermistorB;
    int tankMinResistance; //used
    int tankMaxResistance; //used
    int tauCoolant; //used
    int tauOil;
    int tauAir;
    int tauTank; //used
    uint32_t mileage; //used
    uint16_t daily_mileage; //used
    uint8_t autoBrightness;
    uint8_t brightnessLevel;
    uint8_t tankCapacity;
    uint8_t mfaState;
    uint8_t buzzerOff;
    int maxRPM;
};

void initEEPROM();
bool checkMagicBytes();
void saveParameters();

#endif
