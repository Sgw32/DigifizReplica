#ifndef TACHO_H
#define TACHO_H
#include "Arduino.h"
#include <PinChangeInterrupt.h>

#define RPM_PIN A12 //PK4 //KL_1

void initTacho();
uint16_t readLastRPM();

#endif
