#ifndef MFA_H
#define MFA_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file mfa.h
 * @brief Multi-function display (MFA) user interaction and state API.
 */

#include "params.h"
#include "setup.h"

#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
#define MFA_STATE_TRIP_DURATION 0
#define MFA_STATE_TRIP_DISTANCE 1
#define MFA_STATE_SENSOR   2
#define MFA_STATE_TRIP_MEAN_SPEED 3
#define MFA_STATE_OIL_TEMP 4
#define MFA_STATE_AIR_TEMP 5
#define MFA_STATE_FUEL_PRESSURE 6
#endif

#ifdef AUDI_DISPLAY
#define MFA_STATE_SENSOR   0
#define MFA_STATE_TRIP_CURRENT_L100KM   1
#define MFA_STATE_TRIP_MEAN_SPEED 2
#define MFA_STATE_TRIP_DISTANCE 3
#define MFA_STATE_TRIP_DURATION 4
#define MFA_STATE_TRIP_TIME 5
#endif

#ifdef AUDI_RED_DISPLAY
#define MFA_STATE_TRIP_MEAN_SPEED   0
#define MFA_STATE_SENSOR   1
#define MFA_STATE_TRIP_FUEL 2
#define MFA_STATE_TRIP_MPG 3
#define MFA_STATE_TRIP_MEAN_MPH 4
#define MFA_STATE_TRIP_DISTANCE 5
#define MFA_STATE_TRIP_DURATION 6
#endif

#define MFA_CLOCK_SENSOR_CLOCK 0
#define MFA_CLOCK_SENSOR_BAROMETER 1
#define MFA_CLOCK_SENSOR_LAMBDA_AFR 2
#define MFA_CLOCK_SENSOR_FUEL_PRESSURE 3
#define MFA_CLOCK_SENSOR_MANIFOLD_PRESSURE 4
#define MFA_CLOCK_SENSOR_CURRENT_GEAR 5

#define MFA_SENSOR_CONSUMPTION 0
#define MFA_SENSOR_BAROMETER 1
#define MFA_SENSOR_LAMBDA_AFR 2
#define MFA_SENSOR_FUEL_PRESSURE 3
#define MFA_SENSOR_MANIFOLD_PRESSURE 4
#define MFA_SENSOR_CURRENT_GEAR 5

#define TOUCH_PIN 38

/**
 * @brief Initialize MFA module resources and default state.
 */
void initMFA();

/**
 * @brief Run periodic MFA processing.
 */
void processMFA();

/**
 * @brief Handle MFA mode button press event.
 */
void pressMFAMode();

/**
 * @brief Handle MFA block/select button press event.
 */
void pressMFABlock();

/**
 * @brief Handle MFA reset button press event.
 */
void pressMFAReset();

/**
 * @brief Handle short sensor button press event.
 */
void pressMFASensorShort();

/**
 * @brief Handle long sensor button press event.
 */
void pressMFASensorLong();

/**
 * @brief Handle very long sensor button press event.
 */
void pressMFASensorSuperLong();

/**
 * @brief Handle extra-long sensor button press event.
 */
void pressMFASensorSuperSuperLong();

/**
 * @brief Get active MFA sensor value for current display page.
 *
 * @return float Current sensor value.
 */
float getMFASensorValue();

/**
 * @brief Get active MFA clock-sensor overlay value.
 *
 * @return float Current clock sensor value.
 */
float getMFAClockSensorValue();

/** @brief Enables temporary uptime display page when non-zero. */
extern uint8_t uptimeDisplayEnabled;

#ifdef __cplusplus
}
#endif
#endif
