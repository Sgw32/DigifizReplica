#ifndef DIGIFIZ_TIME_H
#define DIGIFIZ_TIME_H

/**
 * @file digifiz_time.h
 * @brief RTC helper API for setting and printing wall-clock time.
 */

#include <stdint.h>

/**
 * @brief Set RTC hour value.
 *
 * @param hour Hour in 24-hour format (0..23).
 */
void set_hour(uint8_t hour);

/**
 * @brief Set RTC minute value.
 *
 * @param minute Minute value (0..59).
 */
void set_minute(uint8_t minute);

/**
 * @brief Print current RTC time to the active logging interface.
 */
void print_current_time(void);

#endif // DIGIFIZ_TIME_H
