#ifndef MFA_H
#define MFA_H
#include "ext_eeprom.h"
#include "setup.h"

#define MFA_MODE_PIN A13 //PJ0
#define MFA_BLOCK_PIN 15 //PJ0
#define MFA_RESET_PIN 14 //PJ1
#define MFA_SENSOR_PIN 23 //PA1


#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY) && !defined(TRANSPORTER_DISPLAY)
#define MFA_STATE_TRIP_DURATION 0
#define MFA_STATE_TRIP_DISTANCE 1
#define MFA_STATE_TRIP_L100KM   2
#define MFA_STATE_TRIP_MEAN_SPEED 3
#define MFA_STATE_OIL_TEMP 4
#define MFA_STATE_AIR_TEMP 5
#define MFA_STATE_FUEL_PRESSURE 6

#define MFA_STATE_MAX 5
#endif

#if defined(AUDI_DISPLAY)
#define MFA_STATE_TRIP_L100KM   0
#define MFA_STATE_TRIP_CURRENT_L100KM   1
#define MFA_STATE_TRIP_MEAN_SPEED 2
#define MFA_STATE_TRIP_DISTANCE 3
#define MFA_STATE_TRIP_DURATION 4
#define MFA_STATE_TRIP_TIME 5

#define MFA_STATE_MAX 5
#endif

#if defined(TRANSPORTER_DISPLAY)
#define MFA_STATE_TRIP_L100KM   0
#define MFA_STATE_TRIP_MEAN_SPEED 1
#define MFA_STATE_AIR_TEMP 2
#define MFA_STATE_OIL_TEMP 3
#define MFA_STATE_TRIP_DISTANCE 4
#define MFA_STATE_TRIP_DURATION 5
#define MFA_STATE_TRIP_TIME 6

#define MFA_STATE_MAX 6
#endif

#ifdef AUDI_RED_DISPLAY
#define MFA_STATE_TRIP_MEAN_SPEED   0
#define MFA_STATE_TRIP_L100KM   1
#define MFA_STATE_TRIP_FUEL 2
#define MFA_STATE_TRIP_MPG 3
#define MFA_STATE_TRIP_MEAN_MPH 4
#define MFA_STATE_TRIP_DISTANCE 5
#define MFA_STATE_TRIP_DURATION 6

#define MFA_STATE_MAX 6
#endif


/**
 * @brief Initializes MFA inputs, state machine, and persisted settings.
 */
void initMFA();

/**
 * @brief Processes MFA button/sensor state and updates active display mode.
 */
void processMFA();

/**
 * @brief Handles a short press on MFA mode button.
 */
void pressMFAMode();

/**
 * @brief Handles a short press on MFA block button.
 */
void pressMFABlock();

/**
 * @brief Handles a press on MFA reset button.
 */
void pressMFAReset();

/**
 * @brief Handles short touch-sensor interaction.
 */
void pressMFASensorShort();

/**
 * @brief Handles long touch-sensor interaction.
 */
void pressMFASensorLong();

/**
 * @brief Handles extra-long touch-sensor interaction.
 */
void pressMFASensorSuperLong();

/**
 * @brief Handles maximum-duration touch-sensor interaction.
 */
void pressMFASensorSuperSuperLong();

#endif
