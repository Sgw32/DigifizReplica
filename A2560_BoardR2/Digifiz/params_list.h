/**
  ******************************************************************************
  * List of Digifiz parameters for Arduino build
  ******************************************************************************
  */
#ifndef INC_PARAM_LIST_H_
#define INC_PARAM_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "setup.h"

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
        .p_info = "Enable miles per hour support",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_gallons, \
        .p_name = "Gallons option", \
        .p_info = "Enable gallons based consumption",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_fahrenheit, \
        .p_name = "Fahrenheit option", \
        .p_info = "Display temperatures in Fahrenheit",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_kelvin, \
        .p_name = "Kelvin option", \
        .p_info = "Display temperatures in Kelvin",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_lbar, \
        .p_name = "Fuel L/Bar", \
        .p_info = "Switch between fuel units",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_linear_fuel, \
        .p_name = "Linear fuel calculation", \
        .p_info = "Calculate fuel level using linear approximation",\
        .value = 1, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        option_testmode_on, \
        .p_name = "Test mode", \
        .p_info = "Enable test mode",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        rpmOptions_redline_segments, \
        .p_name = "RPM red segments", \
        .p_info = "Number of redline segments",\
        .value = 8, \
        .max = 31, \
    ) \
    PARAM(  \
        U8, \
        rpmOptions_diesel_line, \
        .p_name = "Diesel RPM scale", \
        .p_info = "Enable diesel style RPM gauge",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        rpmOptions_7k_line, \
        .p_name = "7k RPM scale", \
        .p_info = "Enable 7k RPM gauge",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_red_segments, \
        .p_name = "Temp red segments", \
        .p_info = "Number of red segments for temperature",\
        .value = 0, \
        .max = 3, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_sensor_connected_ind, \
        .p_name = "Temp sensor indicator", \
        .p_info = "Display segment when sensor connected",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        tempOptions_alarm_function, \
        .p_name = "Temp alarm", \
        .p_info = "Blink gauge on temperature alarm",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signOptions_use_blink_alt_in, \
        .p_name = "Alt. blink inputs", \
        .p_info = "Use alternative blinker inputs",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signOptions_enable_touch_sensor, \
        .p_name = "Touch sensor", \
        .p_info = "Enable wireless touch sensor",\
        .value = 1, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        signOptions_invert_light_input, \
        .p_name = "Invert light input", \
        .p_info = "Invert brightness control input",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U16, \
        rpmCoefficient, \
        .p_name = "RPM coefficient", \
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
        .p_name = "Coolant beta", \
        .p_info = "Beta coefficient for coolant thermistor",\
        .value = COOLANT_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorB, \
        .p_name = "Oil beta", \
        .p_info = "Beta coefficient for oil thermistor",\
        .value = OIL_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        airThermistorB, \
        .p_name = "Air beta", \
        .p_info = "Beta coefficient for air thermistor",\
        .value = AIR_THERMISTOR_B, \
    ) \
    PARAM(  \
        U16, \
        tankMinResistance, \
        .p_name = "Tank min R", \
        .p_info = "Minimum fuel sensor resistance",\
        .value = 35, \
    ) \
    PARAM(  \
        U16, \
        tankMaxResistance, \
        .p_name = "Tank max R", \
        .p_info = "Maximum fuel sensor resistance",\
        .value = 265, \
    ) \
    PARAM(  \
        U16, \
        tauCoolant, \
        .p_name = "Coolant LPF", \
        .p_info = "Coolant filter coefficient",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauOil, \
        .p_name = "Oil LPF", \
        .p_info = "Oil filter coefficient",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauAir, \
        .p_name = "Air LPF", \
        .p_info = "Air filter coefficient",\
        .value = 2, \
    ) \
    PARAM(  \
        U16, \
        tauTank, \
        .p_name = "Tank LPF", \
        .p_info = "Fuel tank filter coefficient",\
        .value = 2, \
    ) \
    PARAM(  \
        U8, \
        autoBrightness, \
        .p_name = "Auto brightness", \
        .p_info = "Enable automatic brightness",\
        .value = 1, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        brightnessLevel, \
        .p_name = "Brightness level", \
        .p_info = "Manual brightness level",\
        .value = 10, \
    ) \
    PARAM(  \
        U8, \
        tankCapacity, \
        .p_name = "Tank capacity", \
        .p_info = "Fuel tank capacity",\
        .value = 63, \
    ) \
    PARAM(  \
        U8, \
        mfaState, \
        .p_name = "MFA state", \
        .p_info = "Selected MFA function",\
        .value = 0, \
    ) \
    PARAM(  \
        U8, \
        buzzerOff, \
        .p_name = "Buzzer option", \
        .p_info = "Disable embedded buzzer",\
        .value = 1, \
        .max = 1, \
    ) \
    PARAM(  \
        U16, \
        maxRPM, \
        .p_name = "Maximum RPM", \
        .p_info = "Maximum RPM value",\
        .value = 7000, \
    ) \
    PARAM(  \
        U8, \
        mfaBlock, \
        .p_name = "MFA block", \
        .p_info = "Selected MFA block",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        displayDot, \
        .p_name = "Display dot", \
        .p_info = "Blink MFA dot",\
        .value = 0, \
        .max = 1, \
    ) \
    PARAM(  \
        U8, \
        backlight_on, \
        .p_name = "Backlight", \
        .p_info = "Enable backlight power",\
        .value = 1, \
        .max = 1, \
    ) \
    PARAM(  \
        U16, \
        coolantMinResistance, \
        .p_name = "Coolant min", \
        .p_info = "Minimum coolant resistance",\
        .value = 60, \
    ) \
    PARAM(  \
        U16, \
        coolantMaxResistance, \
        .p_name = "Coolant max", \
        .p_info = "Maximum coolant resistance",\
        .value = 120, \
    ) \
    PARAM(  \
        U16, \
        medianDispFilterThreshold, \
        .p_name = "Median filter", \
        .p_info = "Median filter threshold",\
        .value = 65535, \
    ) \
    PARAM(  \
        U16, \
        coolantThermistorDefRes, \
        .p_name = "Coolant R25", \
        .p_info = "Coolant sensor resistance at 25C",\
        .value = COOLANT_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        oilThermistorDefRes, \
        .p_name = "Oil R25", \
        .p_info = "Oil sensor resistance at 25C",\
        .value = OIL_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        ambThermistorDefRes, \
        .p_name = "Ambient R25", \
        .p_info = "Ambient sensor resistance at 25C",\
        .value = AMBIENT_R_AT_NORMAL_T, \
    ) \
    PARAM(  \
        U16, \
        rpmFilterK, \
        .p_name = "RPM filter", \
        .p_info = "RPM filtering coefficient",\
        .value = 70, \
    ) \
    PARAM(  \
        U16, \
        speedFilterK, \
        .p_name = "Speed filter", \
        .p_info = "Speed filtering coefficient",\
        .value = 0, \
    )

#ifdef __cplusplus
}
#endif

#endif /* INC_PARAM_LIST_H_ */
