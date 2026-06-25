#ifndef MILLIS_H
#define MILLIS_H

/**
 * @file millis.h
 * @brief Monotonic millisecond timer helper.
 */

#include <stdint.h>

/**
 * @brief Get milliseconds elapsed since firmware start.
 *
 * @return uint32_t Monotonic uptime in milliseconds.
 */
uint32_t millis(void);

#endif // MILLIS_H
