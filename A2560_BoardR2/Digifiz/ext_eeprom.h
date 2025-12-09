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

// #define OPTION_MFA_MANUFACTURER 1
// #define OPTION_MILES 2
// #define OPTION_GALLONS 4
// #define OPTION_FAHRENHEIT 8
// #define OPTION_KELVIN 16
// #define OPTION_LBAR 32

typedef union DigifizOptions
{
    uint8_t packed_options;       // 1 byte
    struct 
    {
        uint8_t mfa_manufacturer:1;
        uint8_t option_miles:1;
        uint8_t option_gallons:1;
        uint8_t option_fahrenheit:1;
        uint8_t option_kelvin:1;
        uint8_t option_lbar:1;
        uint8_t option_linear_fuel:1;
        uint8_t testmode_on:1;
    };
} u_digifiz_options;
///STATIC_ASSERT(sizeof(u_digifiz_options) == 1, "Size of u_digifiz_options is not 1");

typedef union RPMOptions
{
    uint8_t packed_options;       // 1 byte
    struct 
    {
        uint8_t redline_segments:5; //Red segments in the end
        uint8_t reserved:3;
    };
} u_rpm_options;
///STATIC_ASSERT(sizeof(u_rpm_options) == 1, "Size of u_rpm_options is not 1");

typedef union TemperatureOptions
{
    uint8_t packed_options;       // 1 byte
    struct 
    {
        uint8_t red_segments:2;
        uint8_t sensor_connected_ind:1; //have 1 segment if sensor is connected
        uint8_t alarm_function:1;
        uint8_t reserved:4;
    };
} u_temp_options;
///STATIC_ASSERT(sizeof(u_temp_options) == 1, "Size of u_temp_options is not 1");

typedef union SignalOptions
{
    uint8_t packed_options;       // 1 byte
    struct 
    {
        //Use "FOG" input on Dashboard for left signal, 
        //use "GLASS HEAT" for right signal, disable common
        //Common input becomes dead on rev up to 3.02 if ground is dead
        uint8_t use_blink_other_inputs:1; 
        uint8_t enable_touch_sensor:1; 
        uint8_t invert_light_input:1;
        uint8_t reserved:5;
    };
} u_signal_options;

//STATIC_ASSERT(sizeof(u_signal_options) == 1, "Size of u_signal_options is not 1");


typedef struct digifiz_pars
{
    uint8_t  preamble[4];
    uint32_t mileage; //used //54 //26
    uint32_t daily_mileage[2]; //used //MFA //58 //30
    uint32_t uptime;
    u_digifiz_options digifiz_options; //1 byte
    u_rpm_options rpm_options;             //1 byte
    u_temp_options temp_options;             //1 byte
    u_signal_options sign_options;             //1 byte
    uint16_t rpmQuadraticCoefficient; //div 100000
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
    uint16_t oilThermistorDefRes;   // 2 bytes
    uint16_t ambThermistorDefRes;   // 2 bytes
    uint16_t rpmFilterK;   // 2 bytes
    uint16_t speedFilterK;   // 2 bytes
    uint16_t rpmMaxThreshold;   // 2 bytes
    uint16_t speedMaxThreshold;   // 2 bytes
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
