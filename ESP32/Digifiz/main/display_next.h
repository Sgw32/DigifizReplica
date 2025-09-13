#ifndef DISPLAY_NEXT_H
#define DISPLAY_NEXT_H
#ifdef __cplusplus
extern "C" {
#endif


#include "setup.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "setup.h"
#include <stdbool.h>
#include <string.h>
#ifdef DIGIFIZ_NEXT_DISPLAY

#include "adc.h"
#include "mfa.h"
#include "fuel_pressure.h"

// Turn on debug statements to the serial output
#define DEBUG  1

#define MFA1_PIN 24
#define MFA2_PIN 25

#define BRIGHTNESS_IN_PIN A9 //PK1
#define BACKLIGHT_CTL_PIN 26 //PA4

#define WSDATA_GPIO_PIN 21
#define DIGIFIZ_DISPLAY_NEXT_LEDS 271
#define DIGIFIZ_BACKLIGHT_LEDS 12

#define USE_DISPLAY_LEDS

// Test mode identifiers
#define TEST_MODE_CYCLE  1
#define TEST_MODE_STATIC 2
#define TEST_MODE_COLOR  3

enum 
{
    DIGIT_NUMBER_0 = 0b0111111,
    DIGIT_NUMBER_1 = 0b0011000,
    DIGIT_NUMBER_2 = 0b1110110,
    DIGIT_NUMBER_3 = 0b1111100,
    DIGIT_NUMBER_4 = 0b1011001,
    DIGIT_NUMBER_5 = 0b1101101,
    DIGIT_NUMBER_6 = 0b1101111,
    DIGIT_NUMBER_7 = 0b0111000,
    DIGIT_NUMBER_8 = 0b1111111,
    DIGIT_NUMBER_9 = 0b1111101,
    DIGIT_NUMBER_EMPTY = 0,
    DIGIT_NUMBER_MINUS = 0b1000000,
};

enum
{
    COLOR_SCHEME_INVALID,
    COLOR_SCHEME_SPEEDOMETER,
    COLOR_SCHEME_TIME,
    COLOR_SCHEME_RPM,
    COLOR_SCHEME_RPM_REDLINE,
    COLOR_SCHEME_TEMPERATURE,
    COLOR_SCHEME_FUEL,
    COLOR_SCHEME_REFUEL,
    COLOR_SCHEME_MILEAGE,
    COLOR_SCHEME_BACKLIGHT_RPM,
    COLOR_SCHEME_BACKLIGHT,
    COLOR_SCHEME_SIGNALS,
    COLOR_SCHEME_SIGNAL_LIGHTS,
    COLOR_SCHEME_SIGNAL_FOGLIGHTS1,
    COLOR_SCHEME_SIGNAL_FOGLIGHTS2,
    COLOR_SCHEME_SIGNAL_GLASSHEAT,
    COLOR_SCHEME_SIGNAL_FARLIGHT,
    COLOR_SCHEME_SIGNAL_LEFT_TURN_IND,
    COLOR_SCHEME_SIGNAL_RIGHT_TURN_IND,
    COLOR_SCHEME_SIGNAL_BRAKES_IND,
    COLOR_SCHEME_SIGNAL_OIL_IND,
    COLOR_SCHEME_SIGNAL_BATTERY_IND,
    COLOR_SCHEME_SIGNAL_MFA1_IND,
    COLOR_SCHEME_SIGNAL_MFA2_IND,
    COLOR_SCHEME_SIGNAL_MFA_INDICATORS,
    COLOR_SCHEME_MFA,
    COLOR_SCHEME_UPPER_BACKLIGHT,
    __MAX_COLOR_SCHEME
};

#define COLORING_SCHEME_MAX_ELEMENTS 50
#define COLORING_SCHEME_REDLINING_LIMIT 69

typedef struct RGBColoringElement
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    int16_t end_segment;
    uint8_t type;
    uint8_t basecolor_enabled;
} RGBColoringElement;

typedef struct CompiledColoringScheme
{
    RGBColoringElement scheme[DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS];
} CompiledColoringScheme;

typedef struct ColoringScheme
{
    RGBColoringElement scheme[COLORING_SCHEME_MAX_ELEMENTS];
} ColoringScheme;

typedef struct __attribute__((packed)) DigifizNextDisplay
{
    uint8_t coolant_backlight : 1; //1
    uint16_t coolant_value : 14; //15
    uint8_t vdo_backlight : 1; //16
    uint8_t mfa2_ind : 1; //17 
    uint8_t mfa1_ind : 1; //18
    uint8_t battery_ind : 1; //19
    uint8_t oil_ind : 1; //20
    uint8_t brakes_ind : 1; //21
    uint8_t right_turn_ind : 1; //22
    uint8_t left_turn_ind : 1; //23
    uint8_t farlight_ind : 1; //24
    uint8_t glassheat_ind : 1; //25
    uint8_t foglight_ind1 : 1; //26
    uint8_t foglight_ind2 : 1; //27
    uint8_t lights_on_ind : 1; //28
    uint8_t rpm_padding : 4; //32
    uint8_t rpm[5]; //72
    uint8_t rpm_last :5; //77
    uint32_t rpm_backlight: 18; //95
    uint8_t mileage_digit_1 : 7; //102
    uint8_t mileage_digit_2 : 7; //109
    uint8_t mileage_digit_3 : 7; //116
    uint8_t mileage_digit_4 : 7; //123
    uint8_t mileage_digit_5 : 7; //130
    uint8_t mileage_digit_6 : 7; //137
    uint8_t km_backlight : 1; //138
    uint8_t clock_digit_1 : 7; //145
    uint8_t clock_digit_2 : 7; //152
    uint8_t clock_dot : 2; //154
    uint8_t clock_digit_3 : 7; //161
    uint8_t clock_digit_4 : 7; //168
    uint8_t mfa_indicators : 7; //175
    uint8_t mfa_digit_1 : 7; //182
    uint8_t mfa_digit_2 : 7; //189
    uint8_t mfa_digit_3 : 7; //196
    uint8_t mfa_digit_4 : 7; //203
    uint8_t mfa_dots : 3; //206
    uint8_t fuel_digit_1 : 7; //213
    uint8_t fuel_digit_2 : 7; //220
    uint8_t fuel_ind : 1; //221
    uint8_t fuel_low_ind : 6; //227
    uint8_t speed_digit_1 : 7; //234
    uint8_t speed_digit_2 : 7; //241
    uint8_t speed_digit_3 : 7; //248
    uint8_t speed_digit_s1 : 7; //255
    uint8_t speed_digit_s2 : 7; //262
    uint8_t speed_digit_s3 : 7; //269
    uint8_t km_ind : 1; //270
    uint16_t backlight_leds:12; //282
} DigifizNextDisplay;

void initDisplay(); 
void setRPM(uint32_t rpmdata);
void setClockData(uint8_t clock_hours,uint8_t clock_minutes);
void setMFAClockData(uint8_t mfa_clock_hours,uint8_t mfa_clock_minutes);
void setMFADisplayedNumber(int16_t data);
void setFuel(uint8_t litres);
void setRPMData(uint16_t data);
void setSpeedometerData(uint16_t data);
void setCoolantData(uint16_t data);
void setDot(bool value);
void setFloatDot(bool value);
void setMileage(uint32_t mileage);
void setMFAType(uint8_t mfaType);
void displayMFAType(uint8_t mfaType);
void setMFABlock(uint8_t block);
void setBrightness(uint8_t levels);
void resetBrightness();
void setRefuelSign(bool onoff);
void setCheckEngine(bool onoff);
void setBacklight(bool onoff);
void setServiceDisplayData(uint8_t data);

void compileColorScheme(void);

void fireDigifiz();

void setOilIndicator(bool onoff);
void setBrakesIndicator(bool onoff);
void setHeatLightsIndicator(bool onoff);
void setBackLightsHeatIndicator(bool onoff);
void setBackWindowHeatIndicator(bool onoff);
void processIndicators();

void fillAllSegmentsWithColor(uint8_t r, uint8_t g, uint8_t b);

void deinit_leds(void);

extern ColoringScheme digifizCustom;
extern ColoringScheme digifizStandard;

#ifdef __cplusplus
}
#endif
#endif
#endif
