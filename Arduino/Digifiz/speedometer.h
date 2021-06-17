#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H
#include "Arduino.h"
#include <PinChangeInterrupt.h>

#define SPD_M_PIN PJ3

void initSpeedometer();
uint16_t readLastSpeed();

#endif
