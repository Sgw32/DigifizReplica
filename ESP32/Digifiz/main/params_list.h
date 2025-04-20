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
        .p_name = "MFA Option Enable", \
        .p_info = "Enable MFA steering columns switch",\
        .value = 1, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_miles, \
        .p_name = "Miles option", \
        .p_info = "Enable Miles option",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_gallons, \
        .p_name = "Gallons option", \
        .p_info = "Enable Gallons option",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_fahrenheit, \
        .p_name = "Fahreheit option", \
        .p_info = "Enable Fahrenheit degress option",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_kelvin, \
        .p_name = "Kelvin option", \
        .p_info = "Enable Kelvin degrees option",\
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
        .p_name = "Linear fuel calculation", \
        .p_info = "Calculate the fuel level by simple linear formula",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_testmode_on, \
        .p_name = "Test mode", \
        .p_info = "Enable testmode",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        rpmOptions_redline_segments, \
        .p_name = "Redline segments", \
        .p_info = "Redline segments number",\
        .value = 8, \
        .max = 31, \
    ) \
    PARAM(  \
        U8, \
        rpmOptions_diesel_line, \
        .p_name = "Diesel option", \
        .p_info = "Diesel-style RPM gauge",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_red_segments, \
        .p_name = "Red segments count", \
        .p_info = "Temperature red segments count",\
        .value = 0, \
        .max = 3, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_sensor_connected_ind, \
        .p_name = "Indicate temp. sensor", \
        .p_info = "Display that sensor is connected by indicating 1 segment",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_oil_atten, \
        .p_name = "Lower atten. enable", \
        .p_info = "Enable lower attenuator for Oil channel",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_alarm_function, \
        .p_name = "Temp. alarm function", \
        .p_info = "Blink the temperature gauge if alarm",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_use_blink_alt_in, \
        .p_name = "Other blink", \
        .p_info = "Use alternative separate blinker inputs",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_enable_touch_sensor, \
        .p_name = "Touch sensor option", \
        .p_info = "Enable wireless touch sensor",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_invert_light_input, \
        .p_name = "Invert light signal input", \
        .p_info = "Invert behaviour of brightness adjustment",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signalOptions_enable_consumption_sensor, \
        .p_name = "Consumption sensor option", \
        .p_info = "Enable external consumption MAF sensor",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U16, \
        rpmCoefficient, \
        .p_name = "RPM Coefficient", \
        .p_info = "Coefficient for RPM calibration",\
        .value = 3000, \
    ) \
    PARAM(  \
        U16, \
        speedCoefficient, \
        .p_name = "Speed coefficient", \
        .p_info = "Coefficient for speed calibration",\
        .value = 100, \
    ) \
    PARAM(  \
        U16, \
        coolantThermistorB, \
        .p_name = "Coolant thermistor beta", \
        .p_info = "Beta-coefficient for coolant temperature equation",\
        .value = COOLANT_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorB, \
        .p_name = "Oil thermistor beta", \
        .p_info = "Beta-coefficient for oil temperature equation",\
        .value = OIL_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        airThermistorB, \
        .p_name = "Air thermistor beta", \
        .p_info = "Beta-coefficient for outdoor temperature equation",\
        .value = AIR_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        tankMinResistance, \
        .p_name = "Fuel tank min. resistance", \
        .p_info = "Minimum resistance of fuel sensor",\
        .value = 35, \
    ) \
    PARAM(  \
        U16, \
        tankMaxResistance, \
        .p_name = "Fuel tank max. resistance", \
        .p_info = "Maximum resistance of fuel sensor",\
        .value = 265, \
    ) \
    PARAM(  \
        U16, \
        tauCoolant, \
        .p_name = "Coolant LPF coeff.", \
        .p_info = "Filter coefficient for coolant temperature",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauOil, \
        .p_name = "Oil LPF coeff.", \
        .p_info = "Filter coefficient for oil temperature",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauAir, \
        .p_name = "Air LPF coeff.", \
        .p_info = "Filter coefficient for outdoor temperature",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauTank, \
        .p_name = "Tank LPF coeff.", \
        .p_info = "Filter coefficient for fuel level",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        maxRPM, \
        .p_name = "Maximum RPM", \
        .p_info = "Maximum RPM [8000,7000,6000]",\
        .value = 8000, \
    ) \
    PARAM(  \
        U16, \
        coolantMin, \
        .p_name = "Min. coolant temp.", \
        .p_info = "Minimum coolant temperature(1 segment)",\
        .value = 60, \
    ) \
    PARAM(  \
        U16, \
        coolantMax, \
        .p_name = "Max. coolant temp.", \
        .p_info = "Minimum coolant temperature(max segments)",\
        .value = 120, \
    ) \
    PARAM(  \
        U16, \
        medianDispFilterThreshold, \
        .p_name = "N/a", \
        .p_info = "Not used",\
        .value = 65535, \
    ) \
    PARAM(  \
        U8, \
        autoBrightness, \
        .p_name = "Auto brightness", \
        .p_info = "Enable auto brightness calculation",\
        .value = 1, \
    ) \
    PARAM(  \
        U8, \
        brightnessLevel, \
        .p_name = "Brightness level", \
        .p_info = "Manual brightness level.",\
        .value = 25, \
    ) \
    PARAM(  \
        U8, \
        tankCapacity, \
        .p_name = "Fuel tank capacity", \
        .p_info = "Fuel tank capacity in liters",\
        .value = 63, \
    ) \
    PARAM(  \
        U8, \
        mfaState, \
        .p_name = "MFA function state", \
        .p_info = "Selected MFA function",\
    ) \
    PARAM(  \
        U8, \
        buzzerOff, \
        .p_name = "Buzzer option", \
        .p_info = "Enable or disable embedded buzzer",\
        .value = 1, \
    ) \
    PARAM(  \
        U8, \
        mfaBlock, \
        .p_name = "MFA Block ind.", \
        .p_info = "Selected MFA memory block",\
    ) \
    PARAM(  \
        U8, \
        displayDot, \
        .p_name = "Blinking dot", \
        .p_info = "Display blinking dot",\
    ) \
    PARAM(  \
        U8, \
        backlight_on, \
        .p_name = "Bkclt power", \
        .p_info = "Enable backlight power",\
        .value = 1, \
    ) \
    PARAM(  \
        U8, \
        mainc_r, \
        .p_name = "Main R color", \
        .p_info = "Main theme Red color component",\
        .value = 180, \
    ) \
    PARAM(  \
        U8, \
        mainc_g, \
        .p_name = "Main G color", \
        .p_info = "Main theme Green color component",\
        .value = 240, \
    ) \
    PARAM(  \
        U8, \
        mainc_b, \
        .p_name = "Main B color", \
        .p_info = "Main theme Blue color component",\
        .value = 6, \
    ) \
    PARAM(  \
        U8, \
        backc_r, \
        .p_name = "Back R color", \
        .p_info = "Main theme backlight Red color component",\
        .value = 180, \
    ) \
    PARAM(  \
        U8, \
        backc_g, \
        .p_name = "Back G color", \
        .p_info = "Main theme backlight Green color component",\
        .value = 240, \
    ) \
    PARAM(  \
        U8, \
        backc_b, \
        .p_name = "Back B color", \
        .p_info = "Main theme backlight Blue color component",\
        .value = 6, \
    ) \
    PARAM(  \
        U16, \
        coolantThermistorDefRes, \
        .p_name = "Coolant default resistance", \
        .p_info = "Default coolant sensor resistance on 25 C degrees",\
        .value = COOLANT_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorDefRes, \
        .p_name = "Oil default resistance", \
        .p_info = "Default oil temperature sensor resistance on 25 C degrees",\
        .value = OIL_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        ambThermistorDefRes, \
        .p_name = "Air default resistance", \
        .p_info = "Default outdoor temperature sensor resistance on 25 C degrees",\
        .value = AMBIENT_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        rpmFilterK, \
        .p_name = "RPM LPF coeff.", \
        .p_info = "LPF coefficient for RPM calculation",\
        .value = 70, \
    ) \
    PARAM(  \
        U16, \
        speedFilterK, \
        .p_name = "LPF coefficient for speed calculation", \
        .p_info = "",\
    ) \
    PARAM(  \
        U16, \
        coolantThermistorPullUpRes, \
        .p_name = "Coolant pull-up", \
        .p_info = "Pull-up resistance for coolant temperature sensor",\
        .value = 220, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorPullUpRes, \
        .p_name = "Oil pull-up", \
        .p_info = "Pull-up resistance for oil temperature sensor",\
        .value = 3300, \
    ) \
    PARAM(  \
        U16, \
        ambThermistorPullUpRes, \
        .p_name = "Amb. air pull-up", \
        .p_info = "Pull-up resistance for ambient temperature sensor",\
        .value = 3300, \
    ) \


#ifdef __cplusplus
}
#endif

#endif /* INC_PARAM_LIST_H_ */
