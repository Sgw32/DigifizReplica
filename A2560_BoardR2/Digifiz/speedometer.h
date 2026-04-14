#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H
/**
 * @file speedometer.h
 * @brief Wheel speed acquisition API.
 */

#include "Arduino.h"
#include "PinChangeInterrupt.h"
#include <MedianFilterLib2.h>

/** @brief MCU pin used for speed signal input. */
#define SPD_M_PIN PJ3

/** @brief Initializes interrupt/filter resources for speed calculation. */
void initSpeedometer();

/**
 * @brief Gets the most recent filtered speed value.
 *
 * @return Speed value encoded by the speedometer module.
 */
uint32_t readLastSpeed();

#endif
