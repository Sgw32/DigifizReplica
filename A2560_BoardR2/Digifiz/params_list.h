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

#ifdef MANUFACTURER_MFA_SWITCH
#define DEFAULT_OPTION_MFA_MANUFACTURER 1
#else
#define DEFAULT_OPTION_MFA_MANUFACTURER 0
#endif

#ifdef MILES
#define DEFAULT_OPTION_MILES 1
#else
#define DEFAULT_OPTION_MILES 0
#endif

#ifdef GALLONS
#define DEFAULT_OPTION_GALLONS 1
#else
#define DEFAULT_OPTION_GALLONS 0
#endif

#ifdef FAHRENHEIT
#define DEFAULT_OPTION_FAHRENHEIT 1
#else
#define DEFAULT_OPTION_FAHRENHEIT 0
#endif

#ifdef KELVIN
#define DEFAULT_OPTION_KELVIN 1
#else
#define DEFAULT_OPTION_KELVIN 0
#endif

#ifdef FUEL_LEVEL_EXPERIMENTAL
#define DEFAULT_OPTION_LINEAR_FUEL 0
#else
#define DEFAULT_OPTION_LINEAR_FUEL 1
#endif

#if defined(DIESEL_MODE)
#define DEFAULT_RPM_COEFFICIENT 400
#elif !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY) && !defined(TRANSPORTER_DISPLAY)
#define DEFAULT_RPM_COEFFICIENT 3000
#else
#define DEFAULT_RPM_COEFFICIENT 1500
#endif

#if defined(RPM_8000)
#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY) || defined(TRANSPORTER_DISPLAY)
#define DEFAULT_MAX_RPM 7000
#else
#define DEFAULT_MAX_RPM 8000
#endif
#else
#define DEFAULT_MAX_RPM 7000
#endif

#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY) && !defined(TRANSPORTER_DISPLAY)
#define DEFAULT_RPM_THRESHOLD 3000
#else
#define DEFAULT_RPM_THRESHOLD 1500
#endif

#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY) || defined(TRANSPORTER_DISPLAY)
#define DEFAULT_TANK_CAPACITY 70
#else
#define DEFAULT_TANK_CAPACITY 63
#endif


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
        .value = DEFAULT_OPTION_MFA_MANUFACTURER, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_miles, \
        .p_name = "", \
        .p_info = "",\
        .value = DEFAULT_OPTION_MILES, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_gallons, \
        .p_name = "", \
        .p_info = "",\
        .value = DEFAULT_OPTION_GALLONS, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_fahrenheit, \
        .p_name = "", \
        .p_info = "",\
        .value = DEFAULT_OPTION_FAHRENHEIT, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_kelvin, \
        .p_name = "", \
        .p_info = "",\
        .value = DEFAULT_OPTION_KELVIN, \
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
        .value = DEFAULT_OPTION_LINEAR_FUEL, \
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
        .value = 8, \
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
        tempOptions_oil_atten, \
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
        .value = 1, \
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
        .value = DEFAULT_RPM_COEFFICIENT, \
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
        .value = DEFAULT_MAX_RPM, \
    ) \
    PARAM(  \
        U16, \
        coolantMinResistance, \
        .p_name = "", \
        .p_info = "",\
        .value = 60, \
    ) \
    PARAM(  \
        U16, \
        coolantMaxResistance, \
        .p_name = "", \
        .p_info = "",\
        .value = 120, \
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
        .value = 10, \
        .max = 70, \
    ) \  
    PARAM(  \
        U8, \
        tankCapacity, \
        .p_name = "", \
        .p_info = "",\
        .value = DEFAULT_TANK_CAPACITY, \
    ) \
    PARAM(  \
        U8, \
        mfaState, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
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
        .value = 0, \
    ) \
    PARAM(  \
        U8, \
        displayDot, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        U8, \
        backlight_on, \
        .p_name = "", \
        .p_info = "",\
        .value = 1, \
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
        rpmFallingFilterK, \
        .p_name = "", \
        .p_info = "",\
        .value = 200, \
    ) \
    PARAM(  \
        U16, \
        speedFilterK, \
        .p_name = "", \
        .p_info = "",\
        .value = 500, \
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
        .value = 220, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorSeriesRes, \
        .p_name = "", \
        .p_info = "d",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        ambThermistorPullUpRes, \
        .p_name = "", \
        .p_info = "",\
        .value = 3300, \
    ) \
    PARAM(  \
        U8, \
        stable_rpm_input, \
        .p_name = "", \
        .p_info = "", \
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_refuel_blink, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U32, \
        speed_meas_period, \
        .p_name = "", \
        .p_info = "",\
        .min = 250, \
        .max = 2000, \
        .value = 1000, \
    ) \
    PARAM(  \
        U32, \
        speed_meas_window, \
        .p_name = "", \
        .p_info = "",\
        .min = 250, \
        .max = 2000, \
        .value = 1000, \
    ) \
    PARAM(  \
        U16, \
        rpmMaxThreshold, \
        .p_name = "", \
        .p_info = "=",\
        .value = DEFAULT_RPM_THRESHOLD, \
    ) \
    PARAM(  \
        U16, \
        speedMaxThreshold, \
        .p_name = "", \
        .p_info = "",\
        .value = 1666, \
    ) \
    PARAM(  \
        U32, \
        mileage, \
        .p_name = "", \
        .p_info = "",\
        .value = DEFAULT_MILEAGE*3600L, \
    ) \
    PARAM(  \
        U32, \
        daily_mileage_0, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        U32, \
        daily_mileage_1, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        U32, \
        uptime, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageSpeed_0, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageSpeed_1, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageConsumption_0, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageConsumption_1, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        duration_0, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        duration_1, \
        .p_name = "", \
        .p_info = "",\
        .value = 0, \
    ) \


#ifdef __cplusplus
}
#endif

#endif /* INC_PARAM_LIST_H_ */
