#ifndef SETUP_H
#define SETUP_H

//#define EMULATE_RTC
//#define DISABLE_EEPROM
#define DISABLE_MANUFACTURER_MFA
#define MANUFACTURER_MFA_SWITCH
//#define EMERGENCY_DISABLE_SENSOR_CHECK

#define OIL_RES_10000

//more brightness
#define YELLOW_GREEN_LED 
    
#define NEW_REVISION

#define EEPROM_SAVE_INTERVAL 32

//#define GALLONS
//#define MILES
//#define FAHRENHEIT  
//#define KELVIN

//#define USE_UIOD
//#define UIOD_PARSE_INPUT

#define RPM_8000

#define DEFAULT_MILEAGE   0L //78893L //103256L

//#define TESTMODE
//#define DIGIFIZ_ORIGINAL_DISPLAY

#define COOLANT_R_AT_NORMAL_T 1000.0f //VW OEM

//#define OIL_R_AT_NORMAL_T 10000.0f //Digifiz replica OEM sensors by PHL
#define OIL_R_AT_NORMAL_T 1000.0f //VW OEM
//#define AMBIENT_R_AT_NORMAL_T 10000.0f //Digifiz replica OEM sensors by PHL
#define AMBIENT_R_AT_NORMAL_T 2991.39f //AutoVAZ sensors

#define COOLANT_THERMISTOR_B 4000 //OEM VW
//#define AIR_THERMISTOR_B 4000 //OEM
#define AIR_THERMISTOR_B 3812 //AutoVAZ sensors
#define OIL_THERMISTOR_B 4000 //OEM

#define CRC_FRAGMENT_SIZE 84

#endif
