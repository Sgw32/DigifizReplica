#ifndef SETUP_H
#define SETUP_H
#ifdef __cplusplus
extern "C" {
#endif

#define configTICK_RATE_HZ                           CONFIG_FREERTOS_HZ
#define portTICK_PERIOD_MS              ( ( TickType_t ) 1000 / configTICK_RATE_HZ )

//#define DISABLE_MANUFACTURER_MFA
#define MANUFACTURER_MFA_SWITCH
//#define EMERGENCY_DISABLE_SENSOR_CHECK
//#define DISABLE_SENSOR_BUTTON

#define AVERAGE_CONSUMPTION_L100KM
//#define FUEL_CONSUMPTION_TESTMODE
//#define CURRENT_CONSUMPTION_L100KM
#define FUEL_LEVEL_EXPERIMENTAL
#define OIL_RES_10000
//#define FUEL_PRESSURE_SENSOR

#define EEPROM_SAVE_INTERVAL 32

#define TEMPERATURE_ALARM_PERIOD 250
//#define DEBUG_SLEEP_DISABLE

extern const char TAG[];

// #define GALLONS
// #define MILES
// #define FAHRENHEIT  
//#define KELVIN

#define RPM_8000
#define DIESEL_MODE
#define DEFAULT_MILEAGE   000000L

//#define TESTMODE

#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
#ifndef DISABLE_SENSOR_BUTTON
#define DISABLE_SENSOR_BUTTON
#endif
#endif

#define COOLANT_R_AT_NORMAL_T 1000.0f //VW OEM

//#define OIL_R_AT_NORMAL_T 10000.0f //Digifiz replica OEM sensors by PHL
#define OIL_R_AT_NORMAL_T 1000.0f //VW OEM
//#define AMBIENT_R_AT_NORMAL_T 10000.0f //Digifiz replica OEM sensors by PHL
//#define AMBIENT_R_AT_NORMAL_T 2991.39f //AutoVAZ sensors
#define AMBIENT_R_AT_NORMAL_T 500.0f //AutoVAZ sensors

#define COOLANT_THERMISTOR_B 4000 //OEM VW
//#define AIR_THERMISTOR_B 4000 //If use same sensor
//#define AIR_THERMISTOR_B 3812 //AutoVAZ sensors
//https://golf1wiki.de/index.php/MFA_Au%C3%9Fentemperaturf%C3%BChler
#define AIR_THERMISTOR_B 3600 //171919379A
#define OIL_THERMISTOR_B 4000 //OEM

#define CRC_FRAGMENT_SIZE 84

#define DIGIFIZ_NEXT_DISPLAY

#ifdef __cplusplus
}
#endif
#endif
