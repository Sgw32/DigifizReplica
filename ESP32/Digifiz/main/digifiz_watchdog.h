#ifndef DIGIFIZ_WATCHDOG_H
#define DIGIFIZ_WATCHDOG_H

#include "esp_err.h"

// Define this macro to enable watchdog functionality
// #define WATCHDOG_ENABLED

#ifdef WATCHDOG_ENABLED

// Initializes the watchdog timer
esp_err_t watchdog_init(void);

// Checks if the watchdog timer is currently running
bool watchdog_is_running(void);

// Updates (kicks) the watchdog timer to prevent a reset
void watchdog_update(void);

// Deletes the watchdog timer (e.g., before entering deep sleep)
void watchdog_delete(void);

// Reinitializes the watchdog timer after deep sleep
esp_err_t watchdog_reinit(void);

// Creates a mutex for watchdog synchronization
void watchdog_mutex_create(void);

// Deletes the mutex for watchdog synchronization
void watchdog_mutex_delete(void);

#else

// Define empty macros for the watchdog functions when disabled
#define watchdog_init() (ESP_OK)
#define watchdog_is_running() (false)
#define watchdog_update()
#define watchdog_delete()
#define watchdog_reinit() (ESP_OK)
#define watchdog_mutex_create()
#define watchdog_mutex_delete()

#endif // WATCHDOG_ENABLED

#endif // DIGIFIZ_WATCHDOG_H
