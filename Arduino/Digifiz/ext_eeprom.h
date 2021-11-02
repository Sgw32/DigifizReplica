#ifndef EXT_EEPROM_H
#define EXT_EEPROM_H
#include <Wire.h>
#include <SparkFun_External_EEPROM.h>
#include <EEPROM.h>
#include "Arduino.h"
#include "setup.h"

#define INTERNAL_OFFSET 32
#define EXTERNAL_OFFSET 32


typedef struct digifiz_pars
{
    uint16_t rpmCoefficient; //used //div 100 //16
    uint16_t speedCoefficient; //used //div 100 //32
    int coolantThermistorB; //used //34
    int oilThermistorB; //used //36
    int airThermistorB; //used //38
    int tankMinResistance; //used //40
    int tankMaxResistance; //used //42
    int tauCoolant; //used //44
    int tauOil; //used //46
    int tauAir; //used //48
    int tauTank; //used //50
    uint32_t mileage; //used //54
    uint32_t daily_mileage[2]; //used //MFA //58
    uint8_t autoBrightness;  //59
    uint8_t brightnessLevel; //used //60
    uint8_t tankCapacity; //used //61
    uint8_t mfaState; //used //62
    uint8_t buzzerOff; //used //63
    int maxRPM; //used //65
    uint8_t mfaBlock; //used //66
    float averageConsumption[2]; //used //MFA //70
    float averageSpeed[2]; //used //MFA //74
    int duration[2]; //used //minutes //MFA //76.
    uint8_t displayDot; //used //77
    uint8_t backlight_on; //used //78
    uint16_t coolantMinResistance; //80
    uint16_t coolantMaxResistance; //82
    uint16_t medianDispFilterThreshold; //84 
};

void load_defaults();
void initEEPROM();
bool checkMagicBytes();
void saveParameters();

#endif
