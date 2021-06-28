#ifndef DISPLAY_H
#define DISPLAY_H
#include <MD_MAX72xx.h>
#include <Wire.h>
#include "MS5611.h"

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

#define MFA_AIR_TEMP 0
#define MFA_OIL_TEMP 1
#define MFA_AVERAGE_SPEED 2
#define MFA_AVERAGE_CONSUMPTION 3
#define MFA_DAILY_MILEAGE 4
#define MFA_DRIVING_TIME 5

#define MFA1_PIN 76
#define MFA2_PIN 75

void initDisplay();
void setRPM(int rpmdata);
void blinking();
void blinking2();
void setClockData(uint8_t hours,uint8_t minutes);
void setMFAClockData(uint8_t hours,uint8_t minutes);
void setFuel(uint8_t litres);
void setRPMData(uint16_t data);
void setSpeedometerData(uint16_t data);
void setCoolantData(uint16_t data);
void setDot(bool value);
void setMileage(uint32_t mileage);
void setMFAType(uint8_t mfaType);
void setMFABlock(uint8_t block);

#endif
