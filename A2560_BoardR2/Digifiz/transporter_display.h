#ifndef TRANSPORTER_DISPLAY_H
#define TRANSPORTER_DISPLAY_H

#include "setup.h"

#ifdef TRANSPORTER_DISPLAY

#include "MD_MAX72xx.h"
#include "STLED316S.h"
#include <RTClib.h>
#include <Wire.h>
#include "MS5611.h"
#include "adc.h"
#include "mfa.h"

// Turn on debug statements to the serial output
#define  DEBUG  1

#if  DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTD(x) Serial.println(x, DEC)

#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTD(x)

#endif

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES  2

#define USE_DISPLAY_LEDS

/*
//Tests:
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

#define CLK_PIN2   14  // or SCK
#define DATA_PIN2  15  // or MOSI
#define CS_PIN2    17  // or SS
*/
#define CLK_PIN   10  // or SCK
#define DATA_PIN  12  // or MOSI
#define CS_PIN    11    // or SS

#define CLK_PIN_STLED   7
#define DATA_PIN_STLED  6  
#define STB_SPD_PIN_STLED 8 
#define STB_CLK_PIN_STLED 9

#define  DELAYTIME  100  // in milliseconds

/*#define MFA_AIR_TEMP 0
#define MFA_OIL_TEMP 1
#define MFA_AVERAGE_SPEED 2
#define MFA_AVERAGE_CONSUMPTION 3
#define MFA_DAILY_MILEAGE 4
#define MFA_DRIVING_TIME 5*/

#define MFA1_PIN 24
#define MFA2_PIN 25

#define BRIGHTNESS_IN_PIN A9 //PK1
#define BACKLIGHT_CTL_PIN 26 //PA4

#define BRIGHTNESS_IN_PIN A9 //PK1
#define BACKLIGHT_CTL_PIN 26 //PA4

#define REFUEL_SIGN_DDR DDRJ //PA4
#define REFUEL_SIGN_PORT PORTJ //PA4
#define REFUEL_SIGN_PIN PJ5 //PA4

#define CHECK_ENGINE_PORT_DDR DDRJ //PA4
#define CHECK_ENGINE_PORT PORTJ //PA4
#define CHECK_ENGINE_PIN PJ4 //PA4

#define NBR_OF_DIGIT  6

void initDisplay();
void setRPM(int rpmdata);
void blinking();
void blinking2();
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
void setDailyMileage(uint32_t mileage);
void setAuxDigit(uint8_t digit);
void setMFAType(uint8_t mfaType);
void displayMFAType(uint8_t mfaType);
void setMFABlock(uint8_t block);
void setBrightness(uint8_t levels);
void setRefuelSign(bool onoff);
void setCheckEngine(bool onoff);
void setBacklight(bool onoff);
void setServiceDisplayData(uint8_t data);
void setMilesKMH(bool onoff);
void setLBar(bool onoff);
void setAudiOptions(uint8_t options);

void fireDigifiz();
#endif
#endif
