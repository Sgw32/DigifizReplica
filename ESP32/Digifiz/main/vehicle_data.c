#include "vehicle_data.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Define the default buffer size for JSON string
#define DEFAULT_JSON_BUFFER_SIZE 512

// Structure to hold vehicle data
typedef struct {
    double coolantTempC;
    double oilTempC;
    double ambientTempC;
    double speedKMH;
    double rpm;
    double fuelLevelL;
    double uptimeH;
    double mileageKM;
    double fuelConsumptionL100;
    double intakeVoltageV;
    double fuelPressureBar;
    double speedRaw;
} vehicle_data_t;

// Static instance of vehicle_data
static vehicle_data_t vehicle_data = {
    .coolantTempC = 0.0,
    .oilTempC = 0.0,
    .ambientTempC = 0.0,
    .speedKMH = 0.0,
    .rpm = 0.0,
    .fuelLevelL = 0.0,
    .uptimeH = 0.0,
    .mileageKM = 0.0,
    .fuelConsumptionL100 = 0.0,
    .intakeVoltageV = 0.0,
    .fuelPressureBar = 0.0
};

// Global JSON string and buffer size
char *g_jsonString = NULL;
size_t g_jsonBufferSize = 0;

// FreeRTOS mutex handle
static SemaphoreHandle_t json_mutex = NULL;

/**
 * @brief Ensures that `g_jsonString` has enough space to hold `newSize` bytes.
 *        If not, reallocates the buffer to accommodate the new size.
 * 
 * @param newSize The required size in bytes.
 * @return 0 on success, -1 on failure.
 */
static int ensure_buffer_size(size_t newSize) {
    if (g_jsonBufferSize >= newSize) {
        // Current buffer is sufficient
        return 0;
    }

    size_t newBufferSize = g_jsonBufferSize > 0 ? g_jsonBufferSize : DEFAULT_JSON_BUFFER_SIZE;

    // Double the buffer size until it's large enough
    while (newBufferSize < newSize) {
        newBufferSize *= 2;
    }

    char *temp = realloc(g_jsonString, newBufferSize);
    if (temp == NULL) {
        return -1; // Reallocation failed
    }

    g_jsonString = temp;
    g_jsonBufferSize = newBufferSize;

    return 0;
}

int update_json_string() {
    int ret = 0;

    if (json_mutex == NULL) {
        // Mutex not initialized
        return -1;
    }

    // Take the mutex before accessing shared resources
    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex
        return -1;
    }

    // Create the root JSON object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        ret = -1;
        goto cleanup;
    }

    // Add each parameter to the JSON object with appropriate key names
    if (!cJSON_AddNumberToObject(root, "CoolantTemperature_C", vehicle_data.coolantTempC) ||
        !cJSON_AddNumberToObject(root, "OilTemperature_C", vehicle_data.oilTempC) ||
        !cJSON_AddNumberToObject(root, "AmbientTemperature_C", vehicle_data.ambientTempC) ||
        !cJSON_AddNumberToObject(root, "Speed_KMH", vehicle_data.speedKMH) ||
        !cJSON_AddNumberToObject(root, "Speed_Raw", vehicle_data.speedRaw) ||
        !cJSON_AddNumberToObject(root, "RPM", vehicle_data.rpm) ||
        !cJSON_AddNumberToObject(root, "FuelLevel_L", vehicle_data.fuelLevelL) ||
        !cJSON_AddNumberToObject(root, "Uptime_h", vehicle_data.uptimeH) ||
        !cJSON_AddNumberToObject(root, "Mileage_KM", vehicle_data.mileageKM) ||
        !cJSON_AddNumberToObject(root, "FuelConsumption_L100", vehicle_data.fuelConsumptionL100) ||
        !cJSON_AddNumberToObject(root, "IntakeVoltage_V", vehicle_data.intakeVoltageV) ||
        !cJSON_AddNumberToObject(root, "FuelPressure_bar", vehicle_data.fuelPressureBar)) {
        cJSON_Delete(root);
        ret = -1;
        goto cleanup;
    }

    // Convert the JSON object to an unformatted string
    char *tempJsonString = cJSON_PrintUnformatted(root);

    // Clean up the JSON object as it's no longer needed
    cJSON_Delete(root);

    if (tempJsonString == NULL) {
        ret = -1;
        goto cleanup;
    }

    size_t requiredSize = strlen(tempJsonString) + 1; // Including null terminator

    // Ensure the buffer is large enough
    if (ensure_buffer_size(requiredSize) != 0) {
        free(tempJsonString);
        ret = -1;
        goto cleanup;
    }

    // Copy the new JSON string into the global buffer
    memcpy(g_jsonString, tempJsonString, requiredSize);

    // Null-terminate the string
    g_jsonString[requiredSize - 1] = '\0';

    // Free the temporary JSON string
    free(tempJsonString);

cleanup:
    // Release the mutex
    xSemaphoreGive(json_mutex);

    return ret;
}

int initVehicleJSON() {
    // Create a mutex for thread safety
    json_mutex = xSemaphoreCreateMutex();
    if (json_mutex == NULL) {
        // Failed to create mutex
        return -1;
    }

    // Allocate memory for the JSON string
    g_jsonString = (char *)malloc(DEFAULT_JSON_BUFFER_SIZE);
    if (g_jsonString == NULL) {
        // Memory allocation failed
        vSemaphoreDelete(json_mutex);
        json_mutex = NULL;
        return -1;
    }

    g_jsonString[0] = '\0'; // Initialize as empty string
    g_jsonBufferSize = DEFAULT_JSON_BUFFER_SIZE;

    // Initialize JSON string with default data
    if (update_json_string() != 0) {
        free(g_jsonString);
        g_jsonString = NULL;
        g_jsonBufferSize = 0;
        vSemaphoreDelete(json_mutex);
        json_mutex = NULL;
        return -1;
    }

    return 0;
}

void cleanup_vehicle_data() {
    if (json_mutex == NULL) {
        // Mutex not initialized
        return;
    }

    // Take the mutex before cleanup to ensure no other tasks are using the JSON string
    if (xSemaphoreTake(json_mutex, (TickType_t)10) == pdTRUE) {
        if (g_jsonString != NULL) {
            free(g_jsonString);
            g_jsonString = NULL;
            g_jsonBufferSize = 0;
        }
        // Release the mutex
        xSemaphoreGive(json_mutex);
    }

    // Delete the mutex
    vSemaphoreDelete(json_mutex);
    json_mutex = NULL;
}

/* Getter Implementations */

double get_coolant_temp_c() {
    double temp;

    if (json_mutex == NULL) {
        return vehicle_data.coolantTempC;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.coolantTempC;
    }

    temp = vehicle_data.coolantTempC;

    xSemaphoreGive(json_mutex);

    return temp;
}

double get_oil_temp_c() {
    double temp;

    if (json_mutex == NULL) {
        return vehicle_data.oilTempC;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.oilTempC;
    }

    temp = vehicle_data.oilTempC;

    xSemaphoreGive(json_mutex);

    return temp;
}

double get_ambient_temp_c() {
    double temp;

    if (json_mutex == NULL) {
        return vehicle_data.ambientTempC;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.ambientTempC;
    }

    temp = vehicle_data.ambientTempC;

    xSemaphoreGive(json_mutex);

    return temp;
}

double get_speed_kmh() {
    double speed;

    if (json_mutex == NULL) {
        return vehicle_data.speedKMH;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.speedKMH;
    }

    speed = vehicle_data.speedKMH;

    xSemaphoreGive(json_mutex);

    return speed;
}

double get_speed_raw() {
    double speed;

    if (json_mutex == NULL) {
        return vehicle_data.speedRaw;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.speedRaw;
    }

    speed = vehicle_data.speedRaw;

    xSemaphoreGive(json_mutex);

    return speed;
}

double get_rpm() {
    double rpm;

    if (json_mutex == NULL) {
        return vehicle_data.rpm;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.rpm;
    }

    rpm = vehicle_data.rpm;

    xSemaphoreGive(json_mutex);

    return rpm;
}

double get_fuel_level_l() {
    double level;

    if (json_mutex == NULL) {
        return vehicle_data.fuelLevelL;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.fuelLevelL;
    }

    level = vehicle_data.fuelLevelL;

    xSemaphoreGive(json_mutex);

    return level;
}

double get_uptime_h() {
    double uptime;

    if (json_mutex == NULL) {
        return vehicle_data.uptimeH;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.uptimeH;
    }

    uptime = vehicle_data.uptimeH;

    xSemaphoreGive(json_mutex);

    return uptime;
}

double get_mileage_km() {
    double mileage;

    if (json_mutex == NULL) {
        return vehicle_data.mileageKM;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.mileageKM;
    }

    mileage = vehicle_data.mileageKM;

    xSemaphoreGive(json_mutex);

    return mileage;
}

double get_fuel_consumption_l100() {
    double consumption;

    if (json_mutex == NULL) {
        return vehicle_data.fuelConsumptionL100;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.fuelConsumptionL100;
    }

    consumption = vehicle_data.fuelConsumptionL100;

    xSemaphoreGive(json_mutex);

    return consumption;
}

double get_intake_voltage_v() {
    double voltage;

    if (json_mutex == NULL) {
        return vehicle_data.intakeVoltageV;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.intakeVoltageV;
    }

    voltage = vehicle_data.intakeVoltageV;

    xSemaphoreGive(json_mutex);

    return voltage;
}

double get_fuel_pressure_bar() {
    double pressure;

    if (json_mutex == NULL) {
        return vehicle_data.fuelPressureBar;
    }

    if (xSemaphoreTake(json_mutex, (TickType_t)10) != pdTRUE) {
        // Failed to take the mutex, return current value without synchronization
        return vehicle_data.fuelPressureBar;
    }

    pressure = vehicle_data.fuelPressureBar;

    xSemaphoreGive(json_mutex);

    return pressure;
}

/* Setter Implementations */

int set_coolant_temp_c(double temp) {
    vehicle_data.coolantTempC = temp;
    return 0;
}

int set_oil_temp_c(double temp) {
    vehicle_data.oilTempC = temp;
    return 0;
}

int set_ambient_temp_c(double temp) {
    vehicle_data.ambientTempC = temp;
    return 0;
}

int set_speed_kmh(double speed) {
    vehicle_data.speedKMH = speed;
    return 0;
}

int set_speed_raw(double speed) {
    vehicle_data.speedRaw = speed;
    return 0;
}

int set_rpm(double rpm) {
    vehicle_data.rpm = rpm;
    return 0;
}

int set_fuel_level_l(double level) {
    vehicle_data.fuelLevelL = level;
    return 0;
}

int set_uptime_h(double uptime) {
    vehicle_data.uptimeH = uptime;
    return 0;
}

int set_mileage_km(double mileage) {
    vehicle_data.mileageKM = mileage;
    return 0;
}

int set_fuel_consumption_l100(double consumption) {
    vehicle_data.fuelConsumptionL100 = consumption;
    return 0;
}

int set_intake_voltage_v(double voltage) {
    vehicle_data.intakeVoltageV = voltage;
    return 0;
}

int set_fuel_pressure_bar(double pressure) {
    vehicle_data.fuelPressureBar = pressure;
    return 0;
}
