#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "digifiz_time.h"

static const char *TAG = "DigifizTime";

void set_hour(uint8_t hour) {
    struct tm tm;
    time_t t;

    // Get the current time
    time(&t);
    // Convert time_t to struct tm
    localtime_r(&t, &tm);

    // Set the desired hour
    tm.tm_hour = hour;

    // Convert struct tm back to time_t
    t = mktime(&tm);

    // Set the new time
    struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, NULL);

    ESP_LOGI(TAG, "Hour set to: %s", asctime(&tm));
}

void set_minute(uint8_t minute) {
    struct tm tm;
    time_t t;

    // Get the current time
    time(&t);
    // Convert time_t to struct tm
    localtime_r(&t, &tm);

    // Set the desired minute
    tm.tm_min = minute;

    // Convert struct tm back to time_t
    t = mktime(&tm);

    // Set the new time
    struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, NULL);

    ESP_LOGI(TAG, "Minute set to: %s", asctime(&tm));
}

void print_current_time(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "Current time: %s", asctime(&timeinfo));
}
