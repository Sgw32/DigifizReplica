#ifndef EXT_EEPROM_H
#define EXT_EEPROM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "nvs_flash.h"
#include "nvs.h"
// #include "setup.h"
#include "params_list.h"
#include "xparam.h"

extern const char LOG_TAG[];

typedef struct {
    PARAM_LIST(DECLARE_PARAM)
}digifiz_pars_t;

typedef struct {
    uint32_t mileage;               // 4 bytes
    uint32_t daily_mileage[2];      // 8 bytes
    uint32_t uptime;                // 4 bytes
    float averageSpeed[2];          // 8 bytes
    float averageConsumption[2];    // 8 bytes
    uint16_t duration[2];           // 4 bytes
}digifiz_stats_t;

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

// /**
//  * @brief Save parameters e.g. while driving
//  * 
//  */
void saveParameters();

extern digifiz_pars_t digifiz_parameters;
extern digifiz_stats_t digifiz_status;

#ifdef __cplusplus
}
#endif
#endif
