#ifndef EEPROM_H
#define EEPROM_H
#include <Wire.h>
#include <SparkFun_External_EEPROM.h>
#include "Arduino.h"
#include "setup.h"

typedef struct digifiz_pars
{
    uint16_t rpmCoefficient; //used //div 100
    uint16_t speedCoefficient; //used //div 100
    int coolantThermistorB; //used
    int oilThermistorB; //used
    int airThermistorB; //used
    int tankMinResistance; //used
    int tankMaxResistance; //used
    int tauCoolant; //used
    int tauOil; //used
    int tauAir; //used
    int tauTank; //used
    uint32_t mileage; //used
    uint32_t daily_mileage[2]; //used //MFA
    uint8_t autoBrightness; 
    uint8_t brightnessLevel; //used
    uint8_t tankCapacity; //used
    uint8_t mfaState; //used
    uint8_t buzzerOff; //used
    int maxRPM; //used
    uint8_t mfaBlock;
    float averageConsumption[2]; //used //MFA
    float averageSpeed[2]; //used //MFA
    int duration[2]; //used //minutes //MFA .
    uint8_t displayDot;
    uint8_t backlight_on;
};

void initEEPROM();
bool checkMagicBytes();
void saveParameters();

#endif
