#ifndef EXT_EEPROM_H
#define EXT_EEPROM_H
#include <Wire.h>
#include <SparkFun_External_EEPROM.h>
#include <EEPROM.h>
#include "Arduino.h"
#include "setup.h"
#include "params.h"

#define INTERNAL_OFFSET (32+256)
#define EXTERNAL_OFFSET (32+256)

#define EEPROM_GAP_SIZE 1024
#define EEPROM_DOUBLING 1

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

// #define OPTION_MFA_MANUFACTURER 1
// #define OPTION_MILES 2
// #define OPTION_GALLONS 4
// #define OPTION_FAHRENHEIT 8
// #define OPTION_KELVIN 16
// #define OPTION_LBAR 32

extern const size_t params_blob_size;

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
 * @brief Save parameters e.g. while driving
 *
 */
void saveParameters();

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
