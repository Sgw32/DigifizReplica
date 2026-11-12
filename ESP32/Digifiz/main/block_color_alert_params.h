#ifndef BLOCK_COLOR_ALERT_PARAMS_H
#define BLOCK_COLOR_ALERT_PARAMS_H

#define BLOCK_COLOR_MODE_INFO "0=no change, 1=constant color, 2=blinking"

#define BLOCK_COLOR_FIELDS(PARAM, prefix, title, limit, band, maximum, units) \
    PARAM(U8, prefix##ColorChangeMode, \
        .p_name = title " color mode", .p_info = BLOCK_COLOR_MODE_INFO, .value = 0, .max = 2,) \
    PARAM(U16, prefix##ColorChangeThreshold, \
        .p_name = title " alert threshold", .p_info = units, .value = limit, .max = maximum,) \
    PARAM(U16, prefix##ColorChangeHysteresis, \
        .p_name = title " alert hysteresis", .p_info = units " recovery band", .value = band, .max = maximum,) \
    PARAM(U8, prefix##AlertColor_r, \
        .p_name = title " alert R", .p_info = "Red (0-255)", .value = 255, .max = 255,) \
    PARAM(U8, prefix##AlertColor_g, \
        .p_name = title " alert G", .p_info = "Green (0-255)", .value = 0, .max = 255,) \
    PARAM(U8, prefix##AlertColor_b, \
        .p_name = title " alert B", .p_info = "Blue (0-255)", .value = 0, .max = 255,)

/* Next-only additions. Existing speed threshold/RGB fields keep their NVS keys. */
#define NEXT_BLOCK_COLOR_ALERT_PARAMS(PARAM) \
    PARAM(U16, blockAlertBlinkPeriodMs, \
        .p_name = "Color alert blink period ms", \
        .p_info = "Shared full cycle; half selected color, half alert color", \
        .value = 1000, .min = 100, .max = 10000,) \
    PARAM(U16, speedColorChangeHysteresis, \
        .p_name = "Speed alert hysteresis", .p_info = "Displayed speed units recovery band", \
        .value = 5, .max = 400,) \
    BLOCK_COLOR_FIELDS(PARAM, rpm, "RPM", 6000, 200, 20000, "RPM") \
    BLOCK_COLOR_FIELDS(PARAM, fuel, "Fuel", 10, 2, 99, "Liters (independent of gallons option)") \
    BLOCK_COLOR_FIELDS(PARAM, coolant, "Coolant", 110, 5, 200, "Degrees Celsius") \
    BLOCK_COLOR_FIELDS(PARAM, oil, "Oil MFA", 130, 5, 200, "Degrees Celsius")

#endif
