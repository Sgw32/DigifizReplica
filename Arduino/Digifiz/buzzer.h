#ifndef BUZZER_H
#define BUZZER_H

#include <Wire.h>
#include "Arduino.h"
#include "eeprom.h"

#define BUZZER_PIN 46 //PL3

void initBuzzer();
void buzzerConstantOn();
void buzzerConstantOff();
void buzzerOn();
void buzzerOff();
uint8_t getBuzzerEnabled();
void buzzerToggle();

#endif