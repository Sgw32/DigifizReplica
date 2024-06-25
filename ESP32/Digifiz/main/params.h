#ifndef EXT_EEPROM_H
#define EXT_EEPROM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "setup.h"
#include <assert.h>

// Check if we're using C11 or later
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define USE_STATIC_ASSERT_C11
#endif

// If we're not using C11, fallback to C99 and check for the presence of static_assert
#ifndef USE_STATIC_ASSERT_C11
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
        #define USE_STATIC_ASSERT_C99
    #endif
#endif

// Define a static assertion based on the selected method
#ifdef USE_STATIC_ASSERT_C11
    #define STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(USE_STATIC_ASSERT_C99)
    #define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
    #define STATIC_ASSERT(cond, msg) extern char static_assertion_failed_##msg[(cond) ? 1 : -1]
#endif

#define INTERNAL_OFFSET 32
#define EXTERNAL_OFFSET 32

#define EEPROM_GAP_SIZE 256
#define EEPROM_DOUBLING 3

extern const char LOG_TAG[];

typedef struct __attribute__((packed)) digifiz_pars
{
   uint8_t header[4];              // 4 bytes
    uint32_t mileage;               // 4 bytes
    uint32_t daily_mileage[2];      // 8 bytes
    uint32_t uptime;                // 4 bytes
    uint32_t digifiz_options;       // 4 bytes
    uint16_t rpmCoefficient;        // 2 bytes
    uint16_t speedCoefficient;      // 2 bytes
    uint16_t coolantThermistorB;    // 2 bytes
    uint16_t oilThermistorB;        // 2 bytes
    uint16_t airThermistorB;        // 2 bytes
    uint16_t tankMinResistance;     // 2 bytes
    uint16_t tankMaxResistance;     // 2 bytes
    uint16_t tauCoolant;            // 2 bytes
    uint16_t tauOil;                // 2 bytes
    uint16_t tauAir;                // 2 bytes
    uint16_t tauTank;               // 2 bytes
    uint16_t maxRPM;                // 2 bytes
    uint16_t coolantMin;  // 2 bytes
    uint16_t coolantMax;  // 2 bytes
    uint16_t medianDispFilterThreshold; // 2 bytes
    float averageConsumption[2];    // 8 bytes
    float averageSpeed[2];          // 8 bytes
    uint16_t duration[2];           // 4 bytes
    uint8_t autoBrightness;         // 1 byte
    uint8_t brightnessLevel;        // 1 byte
    uint8_t tankCapacity;           // 1 byte
    uint8_t mfaState;               // 1 byte
    uint8_t buzzerOff;              // 1 byte
    uint8_t mfaBlock;               // 1 byte
    uint8_t displayDot;             // 1 byte
    uint8_t backlight_on;           // 1 byte
    uint8_t mainc_r;              // 1 byte
    uint8_t mainc_g;              // 1 byte
    uint8_t mainc_b;              // 1 byte
    uint8_t crc;                    // 1 byte
    uint16_t coolantThermistorDefRes; // 2 bytes
} digifiz_pars;

// Ensure that the size of digifiz_pars is divisible by 4
STATIC_ASSERT(sizeof(digifiz_pars) % 4 == 0, "Size of digifiz_pars is not divisible by 4");

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
bool checkInternalMagicBytes();

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

extern digifiz_pars digifiz_parameters;

#ifdef __cplusplus
}
#endif
#endif
