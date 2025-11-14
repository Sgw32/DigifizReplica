#ifndef DIGIFIZ_PARAMS_H
#define DIGIFIZ_PARAMS_H

#include <stdint.h>
#include <stddef.h>
#include "params_list.h"
#include "xparam.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    PARAM_LIST(DECLARE_PARAM)
} digifiz_pars_t;

typedef struct {
    uint32_t mileage;
    uint32_t daily_mileage[2];
    uint32_t uptime;
    float averageSpeed[2];
    float averageConsumption[2];
    uint16_t duration[2];
} digifiz_stats_t;

extern digifiz_pars_t digifiz_parameters;
extern const digifiz_pars_t digifiz_default_parameters;
extern digifiz_stats_t digifiz_status;
extern xparam_table_t params_table;
extern const size_t params_blob_size;

#ifdef __cplusplus
}
#endif

#endif
