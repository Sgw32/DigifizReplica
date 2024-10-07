#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "params.h"
#include "setup.h"

//0..64 normal parameters
//128...128+64 read parameters
//223..225 execute commands

typedef enum {
    PARAMETER_ZERO_RESERVED,       // 0
    PARAMETER_SPEEDCOEEFICIENT,     // 1
    PARAMETER_COOLANTTHERMISTORB,   // 2
    PARAMETER_OILTHERMISTORB,       // 3
    PARAMETER_AIRTHERMISTORB,       // 4
    PARAMETER_TANKMINRESISTANCE,    // 5
    PARAMETER_TANKMAXRESISTANCE,    // 6
    PARAMETER_TAU_COOLANT,          // 7
    PARAMETER_TAU_OIL,              // 8
    PARAMETER_TAU_AIR,              // 9
    PARAMETER_TAU_TANK,             // 10
    PARAMETER_MILEAGE,              // 11
    PARAMETER_DAILY_MILEAGE,        // 12
    PARAMETER_AUTO_BRIGHTNESS,      // 13
    PARAMETER_BRIGHTNESS_LEVEL,     // 14
    PARAMETER_TANK_CAPACITY,        // 15
    PARAMETER_MFA_STATE,            // 16
    PARAMETER_BUZZER_OFF,           // 17
    PARAMETER_MAX_RPM,              // 18
    PARAMETER_NORMAL_RESISTANCE_COOLANT,    // 19
    PARAMETER_NORMAL_RESISTANCE_OIL,        // 20
    PARAMETER_NORMAL_RESISTANCE_AMB,        // 21
    PARAMETER_RPMCOEFFICIENT,        // 22
    PARAMETER_DOT_OFF,
    PARAMETER_BACKLIGHT_ON,         // 24
    PARAMETER_M_D_FILTER,           // 25
    PARAMETER_COOLANT_MAX_R,        // 26
    PARAMETER_COOLANT_MIN_R,        // 27
    PARAMETER_COMMAND_MFA_RESET,    // 28
    PARAMETER_COMMAND_MFA_MODE,     // 29
    PARAMETER_COMMAND_MFA_BLOCK,    // 30
    PARAMETER_MAINCOLOR_R,          // 31
    PARAMETER_MAINCOLOR_G,          // 32
    PARAMETER_MAINCOLOR_B,          // 33
    PARAMETER_BACKCOLOR_R,          // 34
    PARAMETER_BACKCOLOR_G,          // 35
    PARAMETER_BACKCOLOR_B,          // 36
    PARAMETER_RPM_FILTER,           // 37
    PARAMETER_SPEED_FILTER,         // 38
    PARAMETER_SET_FUEL_CALC_FUNCTION,         // 39
    PARAMETER_SET_RPM_OPTIONS,         // 40
    PARAMETER_SET_TEMP_OPTIONS,         // 41
    PARAMETER_SET_SIGNAL_OPTIONS,         // 41
    // Additional parameters and functions
    PARAMETER_UPTIME = 123,
    PARAMETER_READ_ADDITION = 128,
    PARAMETER_SET_HOUR = 255,
    PARAMETER_SET_MINUTE = 254,
    PARAMETER_RESET_DAILY_MILEAGE = 253,
    PARAMETER_RESET_DIGIFIZ = 252,
    // Data acquisition
    PARAMETER_GET_ACCUMULATED_UPTIME = 251,
    PARAMETER_GET_COOLANT_TEMPERATURE = 250,
    PARAMETER_GET_OIL_TEMPERATURE = 249,
    PARAMETER_GET_AMBIENT_TEMPERATURE = 248,
    PARAMETER_GET_FUEL_IN_TANK = 247,
    PARAMETER_GET_SPEED = 246,
    PARAMETER_GET_RPM = 245,
    PARAMETER_SET_DAY = 244,
    PARAMETER_SET_MONTH = 243,
    PARAMETER_SET_YEAR = 242,
    PARAMETER_GET_DAY = 241,
    PARAMETER_GET_MONTH = 240,
    PARAMETER_GET_YEAR = 239,
    PARAMETER_GET_HOUR = 238,
    PARAMETER_GET_MINUTE = 237,
    PARAMETER_GET_GPIO_PINS = 236,
    PARAMETER_MEMORY_LOCK = 235,
    PARAMETER_MEMORY_UNLOCK = 234,
    PARAMETER_TOGGLE_MILES = 233,
    PARAMETER_TOGGLE_GALLONS = 232,
    PARAMETER_TOGGLE_FAHRENHEIT = 231,
    PARAMETER_SAVE_PARAMS = 230,
} DigifizProtocol;

// Structure to map parameter names to enum values
typedef struct {
    const char *name;
    DigifizProtocol value;
} ParameterMap;

void initComProtocol();
void changeBLEName();
void protocolParse(char* buf, uint8_t len);
void processData(int par, long value);
void processGPIOPinsValue(long value);

void printLnCString(char* data);
void printLnUINT8(uint8_t val);
void printLnUINT32(uint32_t val);
void printLnFloat(float val);

void clearProtocolBuffer(void);

extern char ws_data_send[];

#endif
