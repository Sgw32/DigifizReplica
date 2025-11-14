#include "params.h"

const digifiz_pars_t digifiz_default_parameters = {
    PARAM_LIST(DEFINE_PARAM)
};

digifiz_pars_t digifiz_parameters = {
    PARAM_LIST(DEFINE_PARAM)
};

digifiz_stats_t digifiz_status = {
    .mileage = DEFAULT_MILEAGE * 3600UL,
    .daily_mileage = {0, 0},
    .uptime = 0,
    .averageSpeed = {0.0f, 0.0f},
    .averageConsumption = {0.0f, 0.0f},
    .duration = {0, 0},
};

xparam_table_t params_table = {
    .params = (xparam_t*)&digifiz_parameters,
    .n_params = XPARAM_COUNT(digifiz_parameters),
};

const size_t params_blob_size = XPARAM_IMAGE_SIZE(XPARAM_COUNT(digifiz_parameters));
