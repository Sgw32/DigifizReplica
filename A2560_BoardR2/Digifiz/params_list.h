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
#elif !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
#define DEFAULT_RPM_COEFFICIENT 3000
#else
#define DEFAULT_RPM_COEFFICIENT 1500
#endif

#if defined(RPM_8000)
#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
#define DEFAULT_MAX_RPM 7000
#else
#define DEFAULT_MAX_RPM 8000
#endif
#else
#define DEFAULT_MAX_RPM 7000
#endif

#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
#define DEFAULT_RPM_THRESHOLD 3000
#else
#define DEFAULT_RPM_THRESHOLD 1500
#endif

#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
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
        .p_name = "MFA Option Enable", \
        .p_info = "Enable MFA steering columns switch",\
        .value = DEFAULT_OPTION_MFA_MANUFACTURER, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_miles, \
        .p_name = "Miles option", \
        .p_info = "Enable Miles option",\
        .value = DEFAULT_OPTION_MILES, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_gallons, \
        .p_name = "Gallons option", \
        .p_info = "Enable Gallons option",\
        .value = DEFAULT_OPTION_GALLONS, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_fahrenheit, \
        .p_name = "Fahreheit option", \
        .p_info = "Enable Fahrenheit degress option",\
        .value = DEFAULT_OPTION_FAHRENHEIT, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_kelvin, \
        .p_name = "Kelvin option", \
        .p_info = "Enable Kelvin degrees option",\
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
        .p_name = "Linear fuel calculation", \
        .p_info = "Calculate the fuel level by simple linear formula",\
        .value = DEFAULT_OPTION_LINEAR_FUEL, \
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
        rpmOptions_7k_line, \
        .p_name = "7k option", \
        .p_info = "7k-style RPM gauge",\
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
        signalOptions_indicator_filter_cycles, \
        .p_name = "Indicator filter cycles", \
        .p_info = "Cycles required to confirm general indicator changes",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        signalOptions_turn_filter_cycles, \
        .p_name = "Turn signal filter cycles", \
        .p_info = "Cycles required to confirm turn indicator changes",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        rpmQuadraticCoefficient, \
        .p_name = "RPM Quadratic coefficient", \
        .p_info = "Quadratic term coefficient for RPM calibration (div 100000)",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        rpmCoefficient, \
        .p_name = "RPM Coefficient", \
        .p_info = "Coefficient for RPM calibration",\
        .value = DEFAULT_RPM_COEFFICIENT, \
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
        .value = DEFAULT_MAX_RPM, \
    ) \
    PARAM(  \
        U16, \
        coolantMinResistance, \
        .p_name = "Coolant min temp.", \
        .p_info = "Minimum coolant temperature(1 segment)",\
        .value = 60, \
    ) \
    PARAM(  \
        U16, \
        coolantMaxResistance, \
        .p_name = "Coolant max temp.", \
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
        .value = 10, \
        .max = 70, \
    ) \
    PARAM(  \
        U16, \
        brightnessSignalMin, \
        .p_name = "Light signal min", \
        .p_info = "ADC value for minimum brightness",\
        .value = 300, \
    ) \
    PARAM(  \
        U16, \
        brightnessSignalMax, \
        .p_name = "Light signal max", \
        .p_info = "ADC value for maximum brightness",\
        .value = 800, \
    ) \
    PARAM(  \
        U8, \
        brightnessMin, \
        .p_name = "Brightness min", \
        .p_info = "Minimum auto brightness level",\
        .value = 6, \
    ) \
    PARAM(  \
        U8, \
        brightnessMax, \
        .p_name = "Brightness max", \
        .p_info = "Maximum auto brightness level",\
        .value = 25, \
    ) \
    PARAM(  \
        U8, \
        brightnessSpeed, \
        .p_name = "Brightness speed", \
        .p_info = "Regulation speed (percent)",\
        .value = 10, \
        .max = 100, \
    ) \
    PARAM(  \
        U8, \
        tankCapacity, \
        .p_name = "Fuel tank capacity", \
        .p_info = "Fuel tank capacity in liters",\
        .value = DEFAULT_TANK_CAPACITY, \
    ) \
    PARAM(  \
        U8, \
        fuel_low_threshold_liters, \
        .p_name = "Low fuel threshold (L)", \
        .p_info = "Fuel level in liters that triggers refuel indicator",\
        .value = 10, \
    ) \
    PARAM(  \
        U8, \
        fuel_critical_threshold_liters, \
        .p_name = "Critical fuel threshold (L)", \
        .p_info = "Fuel level in liters that triggers refuel blinking",\
        .value = 5, \
    ) \
    PARAM(  \
        U8, \
        fuel_low_threshold_gallons, \
        .p_name = "Low fuel threshold (gal)", \
        .p_info = "Fuel level in gallons that triggers refuel indicator",\
        .value = 3, \
    ) \
    PARAM(  \
        U8, \
        fuel_critical_threshold_gallons, \
        .p_name = "Critical fuel threshold (gal)", \
        .p_info = "Fuel level in gallons that triggers refuel blinking",\
        .value = 2, \
    ) \
    PARAM(  \
        U8, \
        mfaState, \
        .p_name = "MFA function state", \
        .p_info = "Selected MFA function",\
        .value = 0, \
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
        .value = 0, \
    ) \
    PARAM(  \
        U8, \
        displayDot, \
        .p_name = "Blinking dot", \
        .p_info = "Display blinking dot",\
        .value = 0, \
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
        speedColorChangeEnable, \
        .p_name = "Speed color alert", \
        .p_info = "Enable speed color change above threshold",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U16, \
        speedColorChangeThreshold, \
        .p_name = "Speed alert threshold", \
        .p_info = "Speed value that enables alert color",\
        .value = 120, \
        .max = 400, \
    ) \
    PARAM(  \
        U8, \
        speedAlertColor_r, \
        .p_name = "Speed alert R", \
        .p_info = "Speed alert color red component",\
        .value = 255, \
    ) \
    PARAM(  \
        U8, \
        speedAlertColor_g, \
        .p_name = "Speed alert G", \
        .p_info = "Speed alert color green component",\
        .value = 0, \
    ) \
    PARAM(  \
        U8, \
        speedAlertColor_b, \
        .p_name = "Speed alert B", \
        .p_info = "Speed alert color blue component",\
        .value = 0, \
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
        U8, \
        ledEffect_type, \
        .p_name = "LED effect", \
        .p_info = "LED effect type",\
        .value = 0, \
    ) \
    PARAM(  \
        U8, \
        ledEffect_hue, \
        .p_name = "Effect hue", \
        .p_info = "Base hue value",\
        .value = 0, \
    ) \
    PARAM(  \
        U8, \
        ledEffect_saturation, \
        .p_name = "Effect saturation", \
        .p_info = "Base saturation",\
        .value = 255, \
    ) \
    PARAM(  \
        U8, \
        ledEffect_value, \
        .p_name = "Effect value", \
        .p_info = "Base brightness",\
        .value = 30, \
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
        rpmFallingFilterK, \
        .p_name = "RPM LPF coeff. fall", \
        .p_info = "LPF coefficient used when RPM decreases",\
        .value = 200, \
    ) \
    PARAM(  \
        U16, \
        speedFilterK, \
        .p_name = "LPF coefficient for speed calculation", \
        .p_info = "",\
        .value = 0, \
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
        .value = 220, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorSeriesRes, \
        .p_name = "Oil series", \
        .p_info = "Series resistance between oil sensor and ground",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        ambThermistorPullUpRes, \
        .p_name = "Amb. air pull-up", \
        .p_info = "Pull-up resistance for ambient temperature sensor",\
        .value = 3300, \
    ) \
    PARAM(  \
        U8, \
        useCustomScheme, \
        .p_name = "Custom scheme", \
        .p_info = "Enable custom display scheme",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        wifi_auto_off, \
        .p_name = "WiFi Auto Off", \
        .p_info = "Turn off WiFi after 1 minute of ignition ON", \
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        stable_rpm_input, \
        .p_name = "Stable RPM input", \
        .p_info = "Enable for modern ECUs with stable RPM signal", \
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U32, \
        r_gear_coefficient, \
        .p_name = "Rear gear coefficient", \
        .p_info = "Proportion between speed and gear. Def. NA",\
        .value = 9999999, \
    ) \
    PARAM(  \
        U32, \
        gear1_coefficient, \
        .p_name = "1-st gear coefficient", \
        .p_info = "Proportion between speed and gear. Def. 020",\
        .value = 1000, \
    ) \
    PARAM(  \
        U32, \
        gear2_coefficient, \
        .p_name = "2-nd gear coefficient", \
        .p_info = "Proportion between speed and gear. Def. 020",\
        .value = 2000, \
    ) \
    PARAM(  \
        U32, \
        gear3_coefficient, \
        .p_name = "3-rd gear coefficient", \
        .p_info = "Proportion between speed and gear. Def. 020",\
        .value = 3000, \
    ) \
    PARAM(  \
        U32, \
        gear4_coefficient, \
        .p_name = "4-th gear coefficient", \
        .p_info = "Proportion between speed and gear. Def. 020",\
        .value = 4000, \
    ) \
    PARAM(  \
        U32, \
        gear5_coefficient, \
        .p_name = "5-th gear coefficient", \
        .p_info = "Proportion between speed and gear. Def. 020",\
        .value = 5000, \
    ) \
    PARAM(  \
        U32, \
        gear6_coefficient, \
        .p_name = "6-th gear coefficient", \
        .p_info = "Proportion between speed and gear. Def. no gear",\
        .value = 10000, \
    ) \
    PARAM(  \
        U8, \
        option_gear_change_indicator, \
        .p_name = "Gear change indicator", \
        .p_info = "Blink gear on speedometer when gear changes",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_gear_indicator_in_refuel, \
        .p_name = "Gear indicator in refuel", \
        .p_info = "Show gear indicator in refuel segment instead of speed display",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_refuel_blink, \
        .p_name = "Blink refuel indicator", \
        .p_info = "Blink refuel (R) indicator when fuel is low",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U32, \
        speed_meas_period, \
        .p_name = "Speedometer measurement period in ms", \
        .p_info = "Measurement period for PCNT. Updates after reboot.",\
        .min = 250, \
        .max = 2000, \
        .value = 1000, \
    ) \
    PARAM(  \
        U32, \
        speed_meas_window, \
        .p_name = "Speedometer measurement window in ms", \
        .p_info = "Measurement period for PCNT. Updates after reboot.",\
        .min = 250, \
        .max = 2000, \
        .value = 1000, \
    ) \
    PARAM(  \
        U8, \
        stable_speed_input, \
        .p_name = "Stable Speed input", \
        .p_info = "Enable for electronic speed signal", \
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        filtered_speed_input, \
        .p_name = "Filtered Speed input", \
        .p_info = "Enable for speed signal filtering(disable stable)", \
        .value = 1, \
        .max = 1, \
    ) \
    PARAM(  \
        U16, \
        speed_min_threshold_value, \
        .p_name = "Minimum speed threshold", \
        .p_info = "Minimum speed shown as non-zero",\
        .value = 5, \
        .max = 400, \
    ) \
    PARAM(  \
        U16, \
        rpmMaxThreshold, \
        .p_name = "RPM alert threshold", \
        .p_info = "Maximum RPM before triggering alert",\
        .value = DEFAULT_RPM_THRESHOLD, \
    ) \
    PARAM(  \
        U16, \
        speedMaxThreshold, \
        .p_name = "Speed alert threshold", \
        .p_info = "Maximum speed before triggering alert",\
        .value = 1666, \
    ) \
    PARAM(  \
        U32, \
        mileage, \
        .p_name = "Mileage (ticks)", \
        .p_info = "Accumulated mileage in seconds",\
        .value = DEFAULT_MILEAGE*3600L, \
    ) \
    PARAM(  \
        U32, \
        daily_mileage_0, \
        .p_name = "Daily mileage A", \
        .p_info = "Daily mileage block A in seconds",\
        .value = 0, \
    ) \
    PARAM(  \
        U32, \
        daily_mileage_1, \
        .p_name = "Daily mileage B", \
        .p_info = "Daily mileage block B in seconds",\
        .value = 0, \
    ) \
    PARAM(  \
        U32, \
        uptime, \
        .p_name = "Uptime", \
        .p_info = "Running time in seconds",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageSpeed_0, \
        .p_name = "Average speed A", \
        .p_info = "Average speed for MFA block A",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageSpeed_1, \
        .p_name = "Average speed B", \
        .p_info = "Average speed for MFA block B",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageConsumption_0, \
        .p_name = "Average consumption A", \
        .p_info = "Average fuel consumption for MFA block A",\
        .value = 0, \
    ) \
    PARAM(  \
        FLOAT, \
        averageConsumption_1, \
        .p_name = "Average consumption B", \
        .p_info = "Average fuel consumption for MFA block B",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        duration_0, \
        .p_name = "Duration A", \
        .p_info = "Trip duration for MFA block A (minutes)",\
        .value = 0, \
    ) \
    PARAM(  \
        U16, \
        duration_1, \
        .p_name = "Duration B", \
        .p_info = "Trip duration for MFA block B (minutes)",\
        .value = 0, \
    ) \


#ifdef __cplusplus
}
#endif

#endif /* INC_PARAM_LIST_H_ */
