#ifndef SETUP_H
#define SETUP_H

//#define EMULATE_RTC
//#define DISABLE_EEPROM
//#define DISABLE_MANUFACTURER_MFA
#define MANUFACTURER_MFA_SWITCH
//#define EMERGENCY_DISABLE_SENSOR_CHECK
//#define DISABLE_SENSOR_BUTTON


//#define AVERAGE_CONSUMPTION_L100KM
//#define FUEL_CONSUMPTION_TESTMODE
#define CURRENT_CONSUMPTION_L100KM

#define OIL_RES_10000

//more brightness
//#define YELLOW_GREEN_LED 
    
#define NEW_REVISION

#define EEPROM_SAVE_INTERVAL 32

#define GALLONS
#define MILES
#define FAHRENHEIT  
//#define KELVIN

//#define USE_UIOD
#define USE_BTSERIAL //default for LED Digifiz Replica, not for UIOD
//#define UIOD_PARSE_INPUT //default for UIOD

//DEFAULT for miniUIOD and BTserial:
#define BTserial Serial
#define UIODserial Serial1

//DEFAULT for miniUIOD if ONLY mUIOD is installed!!!
//#define BTserial Serial1
//#define UIODserial Serial1


//DEFAULT for UIOD Digifiz Replica (LCD one)
//#define BTserial Serial
//#define UIODserial Serial

#define RPM_8000

#define DEFAULT_MILEAGE   000000L //78893L //103256L

//#define TESTMODE
//#define DIGIFIZ_ORIGINAL_DISPLAY
//#define DIGIFIZ_LCD_DISPLAY
//#define AUDI_DISPLAY
//#define AUDI_RED_DISPLAY

#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
#ifndef DISABLE_SENSOR_BUTTON
#define DISABLE_SENSOR_BUTTON
#endif
#endif

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

#define GPIO1_TO_GND_PIN 41
#define GPIO2_TO_GND_PIN 40

#endif
