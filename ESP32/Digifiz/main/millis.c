#include "millis.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

// Function to get the number of milliseconds since the program started
uint32_t millis(void) {
    // Get the number of ticks since the scheduler started
    TickType_t ticks = xTaskGetTickCount();
    // Convert ticks to milliseconds
    return (uint32_t)(ticks * portTICK_PERIOD_MS);
}
