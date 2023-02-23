#ifndef DISPLAY_H
#define DISPLAY_H
#include <MD_MAX72xx.h>

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

#define MFA1_PIN 24
#define MFA2_PIN 25

#define BRIGHTNESS_IN_PIN A9 //PK1
#define BACKLIGHT_CTL_PIN 26 //PA4

void initDisplay();
void setAll(uint8_t lt);

#endif
