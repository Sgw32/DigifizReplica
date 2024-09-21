#ifndef EXT_EEPROM_H
#define EXT_EEPROM_H
#include <Wire.h>
#include <SparkFun_External_EEPROM.h>
#include <EEPROM.h>
#include "Arduino.h"
#include "setup.h"

#define INTERNAL_OFFSET (32+256)
#define EXTERNAL_OFFSET (32+256)

#define EEPROM_GAP_SIZE 256
#define EEPROM_DOUBLING 3

typedef enum
{
    EEPROM_NO_LOAD_ATTEMPT = 555,
    EEPROM_CORRUPTED = 800,
    EEPROM_OK1 = 100,
    EEPROM_OK2 = 200,
    EEPROM_OK3 = 300,
    EEPROM_OK_UNKNOWN = 400,
    EEPROM_OK_TEST = 500,
    EEPROM_CHECKED_CORRUPTED = 900,
} EEPROMLoadResult;

typedef struct digifiz_pars
{
    uint8_t  preamble[4];
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
    uint16_t coolantThermistorDefRes; 
    uint32_t uptime;
    uint32_t digifiz_options; 
    uint8_t crc; //crc
};

#define OPTION_MFA_MANUFACTURER 1
#define OPTION_MILES 2
#define OPTION_GALLONS 4
#define OPTION_FAHRENHEIT 8
#define OPTION_KELVIN 16
#define OPTION_LBAR 32

/**
 * @brief Loads defaults to the memory(internal or external EEPROM)
 * 
 */
void load_defaults();

/**
 * @brief Inits EEPROM (internal or external)
 * 
 */
void initEEPROM();

/**
 * @brief Checks for signature in memory
 * 
 * @return true has signature
 * @return false no signature in memory
 */
bool checkMagicBytes();

/**
 * @brief Save parameters e.g. while driving
 * 
 */
void saveParameters();

/**
 * @brief Computes parameters checksum(not CRC currently)
 * 
 */
void computeCRC();

/**
 * @brief Get the Current Memory Block number
 * 
 * @return uint8_t 
 */
uint8_t getCurrentMemoryBlock();


/**
 * @brief locks memory saving
 * 
 */
void lockMemory();


/**
 * @brief unlocks memory saving
 * 
 */
void unlockMemory();


/**
 * @brief Get the EEPROM load result on boot
 * 
 * @return EEPROMLoadResult 
 */
EEPROMLoadResult getLoadResult();



#endif
