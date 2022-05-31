#ifndef ORIG_DISPLAY_H
#define ORIG_DISPLAY_H
#include "setup.h"

#ifdef DIGIFIZ_ORIGINAL_DISPLAY

#include <RTClib.h>
#include <Wire.h>
#include "MS5611.h"
#include "adc.h"
#include "mfa.h"


#define  DELAYTIME  100  // in milliseconds

#define MFA_AIR_TEMP 0
#define MFA_OIL_TEMP 1
#define MFA_AVERAGE_SPEED 2
#define MFA_AVERAGE_CONSUMPTION 3
#define MFA_DAILY_MILEAGE 4
#define MFA_DRIVING_TIME 5

#define MFA1_PIN 24
#define MFA2_PIN 25

#define BRIGHTNESS_IN_PIN A9 //PK1
#define BACKLIGHT_CTL_PIN 26 //PA4

void initDisplay();
void init_digifiz();
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
void setMFAType(uint8_t mfaType);
void displayMFAType(uint8_t mfaType);
void setMFABlock(uint8_t block);
void setBrightness(uint8_t levels);
void setRefuelSign(bool onoff);
void setCheckEngine(bool onoff);
void setBacklight(bool onoff);

#endif
#endif
