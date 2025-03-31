/**
  ******************************************************************************
  * List of parameters
  ******************************************************************************
  */

#ifndef INC_PARAM_LIST_H_
#define INC_PARAM_LIST_H_
 
#ifdef __cplusplus
extern "C" {
#endif


#include "setup.h"


// typedef union RPMOptions
// {
//     uint8_t packed_options;       // 1 byte
//     struct 
//     {
//         uint8_t redline_segments:5; //Red segments in the end
//         uint8_t reserved:3;
//     };
// } u_rpm_options;
// STATIC_ASSERT(sizeof(u_rpm_options) == 1, "Size of u_rpm_options is not 1");

// typedef union TemperatureOptions
// {
//     uint8_t packed_options;       // 1 byte
//     struct 
//     {
//         uint8_t red_segments:2;
//         uint8_t sensor_connected_ind:1; //have 1 segment if sensor is connected
//         uint8_t alarm_function:1;
//         uint8_t reserved:4;
//     };
// } u_temp_options;
// STATIC_ASSERT(sizeof(u_temp_options) == 1, "Size of u_temp_options is not 1");

// typedef union SignalOptions
// {
//     uint8_t packed_options;       // 1 byte
//     struct 
//     {
//         //Use "FOG" input on Dashboard for left signal, 
//         //use "GLASS HEAT" for right signal, disable common
//         //Common input becomes dead on rev up to 3.02 if ground is dead
//         uint8_t use_blink_other_inputs:1;
//         uint8_t enable_touch_sensor:1;
//         uint8_t invert_light_input:1;
//         uint8_t enable_consumption_sensor:1;
//         uint8_t reserved:4;
//     };
// } u_signal_options;


#define PARAM_LIST(PARAM) \
    PARAM(  \
        U8, \
        option_mfa_manufacturer, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_miles, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_gallons, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_fahrenheit, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_kelvin, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_lbar, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_linear_fuel, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_testmode_on, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        rpmOptions_redline_segments, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 31, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_red_segments, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 3, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_sensor_connected_ind, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_alarm_function, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_use_blink_alt_in, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_enable_touch_sensor, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_invert_light_input, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_enable_consumption_sensor, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U16, \
        rpmCoefficient, \
        .p_name = "", \
        .p_info = "",\
        .value = 3000, \
    ) \
    PARAM(  \
        U16, \
        speedCoefficient, \
        .p_name = "", \
        .p_info = "",\
        .value = 100, \
    ) \
    PARAM(  \
        U16, \
        coolantThermistorB, \
        .p_name = "", \
        .p_info = "",\
        .value = COOLANT_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorB, \
        .p_name = "", \
        .p_info = "",\
        .value = OIL_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        airThermistorB, \
        .p_name = "", \
        .p_info = "",\
        .value = AIR_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        tankMinResistance, \
        .p_name = "", \
        .p_info = "",\
        .value = 35, \
    ) \
    PARAM(  \
        U16, \
        tankMaxResistance, \
        .p_name = "", \
        .p_info = "",\
        .value = 265, \
    ) \
    PARAM(  \
        U16, \
        tauCoolant, \
        .p_name = "", \
        .p_info = "",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauOil, \
        .p_name = "", \
        .p_info = "",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauAir, \
        .p_name = "", \
        .p_info = "",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauTank, \
        .p_name = "", \
        .p_info = "",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        maxRPM, \
        .p_name = "", \
        .p_info = "",\
        .value = 8000, \
    ) \
    PARAM(  \
        U16, \
        coolantMin, \
        .p_name = "", \
        .p_info = "",\
        .value = 60, \
    ) \
    PARAM(  \
        U16, \
        coolantMax, \
        .p_name = "", \
        .p_info = "",\
        .value = 120, \
    ) \
    PARAM(  \
        U16, \
        medianDispFilterThreshold, \
        .p_name = "", \
        .p_info = "",\
        .value = 65535, \
    ) \
    PARAM(  \
        U8, \
        autoBrightness, \
        .p_name = "", \
        .p_info = "",\
        .value = 1, \
    ) \
    PARAM(  \
        U8, \
        brightnessLevel, \
        .p_name = "", \
        .p_info = "",\
        .value = 25, \
    ) \
    PARAM(  \
        U8, \
        tankCapacity, \
        .p_name = "", \
        .p_info = "",\
        .value = 63, \
    ) \
    PARAM(  \
        U8, \
        mfaState, \
        .p_name = "", \
        .p_info = "",\
    ) \
    PARAM(  \
        U8, \
        buzzerOff, \
        .p_name = "", \
        .p_info = "",\
        .value = 1, \
    ) \
    PARAM(  \
        U8, \
        mfaBlock, \
        .p_name = "", \
        .p_info = "",\
    ) \
    PARAM(  \
        U8, \
        displayDot, \
        .p_name = "", \
        .p_info = "",\
    ) \
    PARAM(  \
        U8, \
        backlight_on, \
        .p_name = "", \
        .p_info = "",\
        .value = 1, \
    ) \
    PARAM(  \
        U8, \
        mainc_r, \
        .p_name = "", \
        .p_info = "",\
        .value = 180, \
    ) \
    PARAM(  \
        U8, \
        mainc_g, \
        .p_name = "", \
        .p_info = "",\
        .value = 240, \
    ) \
    PARAM(  \
        U8, \
        mainc_b, \
        .p_name = "", \
        .p_info = "",\
        .value = 6, \
    ) \
    PARAM(  \
        U8, \
        backc_r, \
        .p_name = "", \
        .p_info = "",\
        .value = 180, \
    ) \
    PARAM(  \
        U8, \
        backc_g, \
        .p_name = "", \
        .p_info = "",\
        .value = 240, \
    ) \
    PARAM(  \
        U8, \
        backc_b, \
        .p_name = "", \
        .p_info = "",\
        .value = 6, \
    ) \
    PARAM(  \
        U16, \
        coolantThermistorDefRes, \
        .p_name = "", \
        .p_info = "",\
        .value = COOLANT_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorDefRes, \
        .p_name = "", \
        .p_info = "",\
        .value = OIL_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        ambThermistorDefRes, \
        .p_name = "", \
        .p_info = "",\
        .value = AMBIENT_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        rpmFilterK, \
        .p_name = "", \
        .p_info = "",\
        .value = 70, \
    ) \
    PARAM(  \
        U16, \
        speedFilterK, \
        .p_name = "", \
        .p_info = "",\
    ) \
    PARAM(  \
        U16, \
        coolantThermistorPullUpRes, \
        .p_name = "", \
        .p_info = "",\
        .value = 220, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorPullUpRes, \
        .p_name = "", \
        .p_info = "",\
        .value = 3300, \
    ) \
    PARAM(  \
        U16, \
        ambThermistorPullUpRes, \
        .p_name = "", \
        .p_info = "",\
        .value = 3300, \
    ) \


#ifdef __cplusplus
}
#endif

#endif /* INC_PARAM_LIST_H_ */
