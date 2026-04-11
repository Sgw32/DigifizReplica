#ifndef BUZZER_H
#define BUZZER_H

/**
 * @file buzzer.h
 * @brief Buzzer output control for audible alerts.
 */

#include "params.h"
#include <stdint.h>

#define BUZZER_PIN 46 //PL3

/**
 * @brief Initialize buzzer GPIO/peripheral resources.
 */
void initBuzzer();

/**
 * @brief Force buzzer pin to continuously enabled state.
 */
void buzzerConstantOn();

/**
 * @brief Force buzzer pin to continuously disabled state.
 */
void buzzerConstantOff();

/**
 * @brief Turn buzzer on if buzzer feature is enabled in settings.
 */
void buzzerOn();

/**
 * @brief Turn buzzer output off.
 */
void buzzerOff();

/**
 * @brief Get current software enabled flag for buzzer notifications.
 *
 * @return uint8_t Non-zero if enabled, 0 if muted.
 */
uint8_t getBuzzerEnabled();

/**
 * @brief Toggle buzzer enabled/disabled setting.
 */
void buzzerToggle();

#endif
