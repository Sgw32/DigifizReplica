#ifndef MFA_H
#define MFA_H
#ifdef __cplusplus
extern "C" {
#endif

#include "params.h"
#include "setup.h"

#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
#define MFA_STATE_TRIP_DURATION 0
#define MFA_STATE_TRIP_DISTANCE 1
#define MFA_STATE_TRIP_L100KM   2
#define MFA_STATE_TRIP_MEAN_SPEED 3
#define MFA_STATE_OIL_TEMP 4
#define MFA_STATE_AIR_TEMP 5
#define MFA_STATE_FUEL_PRESSURE 6
#endif

#ifdef AUDI_DISPLAY
#define MFA_STATE_TRIP_L100KM   0
#define MFA_STATE_TRIP_CURRENT_L100KM   1
#define MFA_STATE_TRIP_MEAN_SPEED 2
#define MFA_STATE_TRIP_DISTANCE 3
#define MFA_STATE_TRIP_DURATION 4
#define MFA_STATE_TRIP_TIME 5
#endif

#ifdef AUDI_RED_DISPLAY
#define MFA_STATE_TRIP_MEAN_SPEED   0
#define MFA_STATE_TRIP_L100KM   1
#define MFA_STATE_TRIP_FUEL 2
#define MFA_STATE_TRIP_MPG 3
#define MFA_STATE_TRIP_MEAN_MPH 4
#define MFA_STATE_TRIP_DISTANCE 5
#define MFA_STATE_TRIP_DURATION 6
#endif

#define TOUCH_PIN 38

void initMFA();
void processMFA();
void pressMFAMode();
void pressMFABlock();
void pressMFAReset();
void pressMFASensorShort();
void pressMFASensorLong();
void pressMFASensorSuperLong();
void pressMFASensorSuperSuperLong();

extern uint8_t uptimeDisplayEnabled;

#ifdef __cplusplus
}
#endif
#endif
