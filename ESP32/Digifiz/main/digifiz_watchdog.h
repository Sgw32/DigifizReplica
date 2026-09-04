#ifndef DIGIFIZ_WATCHDOG_H
#define DIGIFIZ_WATCHDOG_H

/**
 * @file digifiz_watchdog.h
 * @brief Optional watchdog abstraction with no-op fallbacks.
 */

#include "esp_err.h"

/** @brief Define to compile real watchdog implementation instead of no-op macros. */
// #define WATCHDOG_ENABLED

#ifdef WATCHDOG_ENABLED

/** @brief Initialize watchdog timer resources. */
esp_err_t watchdog_init(void);

/** @brief Check whether watchdog is currently armed and running. */
bool watchdog_is_running(void);

/** @brief Feed watchdog to prevent timeout reset. */
void watchdog_update(void);

/** @brief Deinitialize watchdog timer (for example before deep sleep). */
void watchdog_delete(void);

/** @brief Reinitialize watchdog after wake-up from deep sleep. */
esp_err_t watchdog_reinit(void);

/** @brief Create synchronization primitive for watchdog access. */
void watchdog_mutex_create(void);

/** @brief Delete synchronization primitive created for watchdog access. */
void watchdog_mutex_delete(void);

#else

/** @brief No-op watchdog init when watchdog feature is disabled. */
#define watchdog_init() (ESP_OK)
/** @brief Always report watchdog stopped when feature is disabled. */
#define watchdog_is_running() (false)
/** @brief No-op watchdog feed when feature is disabled. */
#define watchdog_update()
/** @brief No-op watchdog delete when feature is disabled. */
#define watchdog_delete()
/** @brief No-op watchdog reinit when feature is disabled. */
#define watchdog_reinit() (ESP_OK)
/** @brief No-op mutex creation when feature is disabled. */
#define watchdog_mutex_create()
/** @brief No-op mutex deletion when feature is disabled. */
#define watchdog_mutex_delete()

#endif // WATCHDOG_ENABLED

#endif // DIGIFIZ_WATCHDOG_H
