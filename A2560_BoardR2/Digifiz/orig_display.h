#ifndef ORIG_DISPLAY_H
#define ORIG_DISPLAY_H
/**
 * @file orig_display.h
 * @brief Display module interface for the A2560 Digifiz firmware.
 */
#include "setup.h"

#ifdef DIGIFIZ_ORIGINAL_DISPLAY

#include <RTClib.h>
#include <Wire.h>
#include "MS5611.h"
#include "adc.h"
#include "mfa.h"


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

/** @brief Initialize display hardware and state. */
void initDisplay();
/** @brief Initialize Digifiz-specific LCD state. */
void init_digifiz();
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

#endif
#endif
