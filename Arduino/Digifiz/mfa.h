#ifndef MFA_H
#define MFA_H
#include "eeprom.h"
#include "setup.h"

#define MFA_MODE_PIN A13 //PJ0
#define MFA_BLOCK_PIN 15 //PJ0
#define MFA_RESET_PIN 14 //PJ1
#define MFA_SENSOR_PIN 23 //PA1

#define MFA_STATE_TRIP_DURATION 0
#define MFA_STATE_TRIP_DISTANCE 1
#define MFA_STATE_TRIP_L100KM   2
#define MFA_STATE_TRIP_MEAN_SPEED 3
#define MFA_STATE_OIL_TEMP 4
#define MFA_STATE_AIR_TEMP 5

void initMFA();
void processMFA();
void pressMFAMode();
void pressMFABlock();
void pressMFAReset();
void pressMFASensorShort();
void pressMFASensorLong();
void pressMFASensorSuperLong();

#endif
