#ifndef XPARAM_EEPROM_H
#define XPARAM_EEPROM_H
/**
 * @file xparam_eeprom.h
 * @brief EEPROM-backed Digifiz parameter table declarations.
 */

#include "xparam.h"
#include "params_list.h"

/**
 * @brief Persistent parameter storage structure generated from PARAM_LIST.
 */
typedef struct
{
    PARAM_LIST(DECLARE_PARAM)
} digifiz_pars;

/** @brief Runtime descriptor table used by xparam helpers. */
extern xparam_table_t params_table;

#endif
