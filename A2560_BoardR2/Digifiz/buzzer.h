#ifndef BUZZER_H
#define BUZZER_H

#include <Wire.h>
#include "Arduino.h"
#include "ext_eeprom.h"

#define BUZZER_PIN 46 //PL3

/**
 * @brief Configures the buzzer GPIO and loads current user preference.
 */
void initBuzzer();

/**
 * @brief Forces the buzzer into a continuous ON state.
 */
void buzzerConstantOn();

/**
 * @brief Forces the buzzer into a continuous OFF state.
 */
void buzzerConstantOff();

/**
 * @brief Starts a short buzzer pulse according to firmware timing.
 */
void buzzerOn();

/**
 * @brief Stops an active buzzer pulse.
 */
void buzzerOff();

/**
 * @brief Returns whether buzzer notifications are enabled.
 *
 * @return uint8_t Non-zero if enabled, 0 if disabled.
 */
uint8_t getBuzzerEnabled();

/**
 * @brief Toggles persisted buzzer enable/disable state.
 */
void buzzerToggle();

#endif
