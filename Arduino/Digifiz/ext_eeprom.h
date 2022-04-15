#ifndef EXT_EEPROM_H
#define EXT_EEPROM_H
#include <Wire.h>
#include <SparkFun_External_EEPROM.h>
#include <EEPROM.h>
#include "Arduino.h"
#include "setup.h"

#define INTERNAL_OFFSET 32
#define EXTERNAL_OFFSET 32

#define EEPROM_GAP_SIZE 256
#define EEPROM_DOUBLING 3

typedef struct digifiz_pars
{
    uint16_t rpmCoefficient; //used //div 100 //16 //2
    uint16_t speedCoefficient; //used //div 100 //32 //4
    uint16_t coolantThermistorB; //used //34 //6
    uint16_t oilThermistorB; //used //36 //8
    uint16_t airThermistorB; //used //38 //10
    uint16_t tankMinResistance; //used //40 //12
    uint16_t tankMaxResistance; //used //42 //14
    uint16_t tauCoolant; //used //44 //16
    uint16_t tauOil; //used //46 //18
    uint16_t tauAir; //used //48 //20
    uint16_t tauTank; //used //50 //22
    uint32_t mileage; //used //54 //26
    uint32_t daily_mileage[2]; //used //MFA //58 //30
    uint8_t autoBrightness;  //59 //31
    uint8_t brightnessLevel; //used //60 //32
    uint8_t tankCapacity; //used //61 //33
    uint8_t mfaState; //used //62 //34
    uint8_t buzzerOff; //used //63 //35
    uint16_t maxRPM; //used //65 //37
    uint8_t mfaBlock; //used //66 //38
    float averageConsumption[2]; //used //MFA //70 //42
    float averageSpeed[2]; //used //MFA //74 //46
    uint16_t duration[2]; //used //minutes //MFA //76 //48
    uint8_t displayDot; //used //77 //49
    uint8_t backlight_on; //used //78 //50
    uint16_t coolantMinResistance; //80 //52
    uint16_t coolantMaxResistance; //82 //54
    uint16_t medianDispFilterThreshold; //84 //56 
    uint8_t crc; //crc
    uint16_t coolantThermistorDefRes; 
    uint32_t uptime;
    uint8_t manufMFAEnabled; 
};

void load_defaults();
void initEEPROM();
bool checkMagicBytes();
void saveParameters();
void computeCRC();
uint8_t getCurrentMemoryBlock();

#endif
