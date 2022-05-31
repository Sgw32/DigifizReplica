#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H
#include "Arduino.h"
#include <PinChangeInterrupt.h>
#include <MedianFilterLib2.h>

#define SPD_M_PIN PJ3

void initSpeedometer();
uint32_t readLastSpeed();

#endif
