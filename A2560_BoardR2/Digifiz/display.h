#ifndef DISPLAY_H
#define DISPLAY_H

#include "setup.h"

#ifndef DIGIFIZ_ORIGINAL_DISPLAY
#ifndef DIGIFIZ_LCD_DISPLAY
#ifndef AUDI_DISPLAY
#ifndef AUDI_RED_DISPLAY
#ifndef TRANSPORTER_DISPLAY

#include "MD_MAX72xx.h"
#include <RTClib.h>
#include <Wire.h>
#include "MS5611.h"
#include "adc.h"
#include "mfa.h"
#include "fuel_pressure.h"

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

#define CLK_PIN2   6  // or SCK
#define DATA_PIN2  8  // or MOSI
#define CS_PIN2    7  // or SS

#define  DELAYTIME  100  // in milliseconds

/*#define MFA_AIR_TEMP 0
#define MFA_OIL_TEMP 1
#define MFA_AVERAGE_SPEED 2
#define MFA_AVERAGE_CONSUMPTION 3
#define MFA_DAILY_MILEAGE 4
#define MFA_DRIVING_TIME 5
*/

#define MFA1_PIN 24
#define MFA2_PIN 25

#define BRIGHTNESS_IN_PIN A9 //PK1
#define BACKLIGHT_CTL_PIN 26 //PA4

#define USE_DISPLAY_LEDS

/**
 * @brief Initializes display drivers, buffers, and startup animation state.
 */
void initDisplay(); 
/// @brief Updates tachometer pointer/segments from engine RPM.
void setRPM(int rpmdata);
/// @brief Writes current time into the primary clock area.
void setClockData(uint8_t clock_hours,uint8_t clock_minutes);
/// @brief Writes current time into the MFA clock area.
void setMFAClockData(uint8_t mfa_clock_hours,uint8_t mfa_clock_minutes);
/// @brief Displays a signed numeric value on MFA segment area.
void setMFADisplayedNumber(int16_t data);
/// @brief Displays fuel quantity.
void setFuel(uint8_t litres);
/// @brief Displays raw RPM value digits.
void setRPMData(uint16_t data);
/// @brief Displays current vehicle speed digits.
void setSpeedometerData(uint16_t data);
/// @brief Displays coolant temperature indicator.
void setCoolantData(uint16_t data);
/// @brief Enables/disables the primary decimal separator.
void setDot(bool value);
/// @brief Enables/disables the auxiliary decimal separator.
void setFloatDot(bool value);
/// @brief Displays odometer mileage.
void setMileage(uint32_t mileage);
/// @brief Selects current MFA item type.
void setMFAType(uint8_t mfaType);
/// @brief Renders current MFA item type label/icon.
void displayMFAType(uint8_t mfaType);
/// @brief Selects current MFA block.
void setMFABlock(uint8_t block);
/// @brief Sets display brightness level.
void setBrightness(uint8_t levels);
/// @brief Shows/hides refuel indicator.
void setRefuelSign(bool onoff);
/// @brief Shows/hides check-engine indicator.
void setCheckEngine(bool onoff);
/// @brief Controls LCD/cluster backlight output.
void setBacklight(bool onoff);
/// @brief Displays service/status byte in service area.
void setServiceDisplayData(uint8_t data);
/// @brief Triggers display self-test/startup effect.
void fireDigifiz();
#endif
#endif
#endif
#endif
#endif
#endif
