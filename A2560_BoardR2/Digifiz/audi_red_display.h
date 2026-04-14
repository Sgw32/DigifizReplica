#ifndef AUDI_RED_DISPLAY_H
#define AUDI_RED_DISPLAY_H

/**
 * @file audi_red_display.h
 * @brief Display module interface for the A2560 Digifiz firmware.
 */
#include "setup.h"

#ifdef AUDI_RED_DISPLAY

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

/*#define MFA_AVERAGE_SPEED 0
#define MFA_AVERAGE_CONSUMPTION 1
#define MFA_FUEL 2
#define MFA_MPG 3
#define MFA_AVERAGE_MPH 4
#define MFA_DAILY_MILEAGE 5
#define MFA_DRIVING_TIME 6*/

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

/** @brief Initialize display hardware and state. */
void initDisplay();
/** @brief Update RPM bar/needle animation source. */
void setRPM(int rpmdata);
/** @brief Advance primary blink state machine. */
void blinking();
/** @brief Advance secondary blink state machine. */
void blinking2();
/** @brief Set clock digits shown on display. */
void setClockData(uint8_t clock_hours,uint8_t clock_minutes);
/** @brief Set MFA clock digits. */
void setMFAClockData(uint8_t mfa_clock_hours,uint8_t mfa_clock_minutes);
/** @brief Set numeric value shown in MFA area. */
void setMFADisplayedNumber(int16_t data);
/** @brief Set fuel level indicator in liters. */
void setFuel(uint8_t litres);
/** @brief Set numeric RPM value output. */
void setRPMData(uint16_t data);
/** @brief Set numeric speed output. */
void setSpeedometerData(uint16_t data);
/** @brief Set coolant temperature field. */
void setCoolantData(uint16_t data);
/** @brief Enable or disable decimal dot. */
void setDot(bool value);
/** @brief Enable or disable floating dot indicator. */
void setFloatDot(bool value);
/** @brief Set total mileage value. */
void setMileage(uint32_t mileage);
/** @brief Set trip/daily mileage value. */
void setDailyMileage(uint32_t mileage);
/** @brief Set auxiliary digit value. */
void setAuxDigit(uint8_t digit);
/** @brief Select MFA metric type. */
void setMFAType(uint8_t mfaType);
/** @brief Render MFA metric icon/text. */
void displayMFAType(uint8_t mfaType);
/** @brief Select MFA block page. */
void setMFABlock(uint8_t block);
/** @brief Set display brightness level. */
void setBrightness(uint8_t levels);
/** @brief Control refuel warning indicator. */
void setRefuelSign(bool onoff);
/** @brief Control check-engine indicator. */
void setCheckEngine(bool onoff);
/** @brief Control display backlight output. */
void setBacklight(bool onoff);
/** @brief Set service display segment data. */
void setServiceDisplayData(uint8_t data);
/** @brief Toggle miles/kmh unit indicator. */
void setMilesKMH(bool onoff);
/** @brief Toggle liter/bar labeling indicator. */
void setLBar(bool onoff);
/** @brief Apply packed display option bits. */
void setAudiOptions(uint8_t options);

/** @brief Run startup animation/test pattern. */
void fireDigifiz();
#endif
#endif
