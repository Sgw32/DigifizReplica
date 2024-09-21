#include "digifiz_watchdog.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define GLOBAL_WDT_TIMEOUT_S 10  // Watchdog timeout in seconds

static bool watchdog_initialized = false;  // Track if the watchdog has been initialized
static SemaphoreHandle_t watchdog_mutex = NULL;  // Mutex handle for watchdog synchronization

#ifdef WATCHDOG_ENABLED

esp_err_t watchdog_init(void)
{
    if (watchdog_initialized) {
        return ESP_ERR_INVALID_STATE;  // Watchdog already initialized
    }

    // Initialize the Task Watchdog Timer
    esp_task_wdt_init(GLOBAL_WDT_TIMEOUT_S, true);
    // Add the main task to the watchdog
    esp_task_wdt_add(NULL);

    watchdog_initialized = true;

    return ESP_OK;
}

bool watchdog_is_running(void)
{
    return watchdog_initialized;
}

void watchdog_update(void)
{
    if (watchdog_initialized) {
        if (xSemaphoreTake(watchdog_mutex, portMAX_DELAY)) {
            esp_task_wdt_reset();  // Reset the watchdog to prevent a system reset
            xSemaphoreGive(watchdog_mutex);
        }
    }
}

void watchdog_delete(void)
{
    if (watchdog_initialized) {
        if (xSemaphoreTake(watchdog_mutex, portMAX_DELAY)) {
            esp_task_wdt_delete(NULL);  // Remove the main task from the watchdog
            watchdog_initialized = false;  // Update the initialization state
            xSemaphoreGive(watchdog_mutex);
        }
    }
}

esp_err_t watchdog_reinit(void)
{
    // Reinitialize the watchdog timer
    return watchdog_init();
}

void watchdog_mutex_create(void)
{
    if (watchdog_mutex == NULL) {
        watchdog_mutex = xSemaphoreCreateMutex();
    }
}

void watchdog_mutex_delete(void)
{
    if (watchdog_mutex != NULL) {
        vSemaphoreDelete(watchdog_mutex);
        watchdog_mutex = NULL;
    }
}

#else

// // Define empty functions when watchdog is disabled
// esp_err_t watchdog_init(void) { return ESP_OK; }
// bool watchdog_is_running(void) { return false; }
// void watchdog_update(void) {}
// void watchdog_delete(void) {}
// esp_err_t watchdog_reinit(void) { return ESP_OK; }
// void watchdog_mutex_create(void) {}
// void watchdog_mutex_delete(void) {}

#endif // WATCHDOG_ENABLED
