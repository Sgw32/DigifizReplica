#ifndef EMERGENCY_H
#define EMERGENCY_H

#include "Arduino.h"
#include <Wire.h>
#include "Arduino.h"
#include "ext_eeprom.h"
#include "buzzer.h"
#include "display.h"
#include "orig_display.h"
#include "lcd_display.h"

#define OIL_0_3BAR_PIN A1 //PF1
#define OIL_1_8BAR_PIN A2 //PF2
#define CHECK_ENGINE_IN 65 //PJ2
#define OIL_LED_PIN 22 //PA0

void initEmergencyModule();
uint8_t processCHECKEngine();
uint8_t processOilPressure(int mRPM);
void checkEmergency(int mRPM);

#endif
