#include "gear_estimator.h"
#include "ble_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

#define STATIC_SPEED_GEAR_INDEX 2
#define DYNAMIC_INITIAL_GEAR_INDEX 2
#define RPM_SHIFT_MIN_DELTA 200.0f
#define RPM_SHIFT_SAMPLE_MS 50
#define RPM_SHIFT_HOLDOFF_MS 400

typedef struct {
    float rpm;
    float speed_kmh;
    float ratio; // computed as rpm / speed
} GearEstimatorInput;

static float gear_coeffs[MAX_GEARS] = {0}; // estimated coefficients
static int calibration_counts[MAX_GEARS] = {0};
static float calibration_sums[MAX_GEARS] = {0};

static GearEstimatorInput current_input = {0};
static volatile float emulator_rpm = 0.0f;
static volatile float emulator_speed = 0.0f;
static volatile int emulator_gear_index = DYNAMIC_INITIAL_GEAR_INDEX;
static unsigned char emulator_mode = 0;

static void gear_speed_emulator_task(void *arg) {
    float previous_rpm = 0.0f;
    TickType_t last_shift = 0;

    while (1) {
        const float rpm = emulator_rpm;
        const TickType_t now = xTaskGetTickCount();
        const float delta = rpm - previous_rpm;

        if (previous_rpm > 300.0f && rpm > 300.0f &&
            (now - last_shift) >= pdMS_TO_TICKS(RPM_SHIFT_HOLDOFF_MS)) {
            if (delta <= -RPM_SHIFT_MIN_DELTA && emulator_gear_index < MAX_GEARS - 1) {
                emulator_gear_index++;
                last_shift = now;
            } else if (delta >= RPM_SHIFT_MIN_DELTA && emulator_gear_index > 0) {
                emulator_gear_index--;
                last_shift = now;
            }
        }

        emulator_speed = rpm > 0.0f ? rpm * gear_coeffs[emulator_gear_index] : 0.0f;
        previous_rpm = rpm;
        vTaskDelay(pdMS_TO_TICKS(RPM_SHIFT_SAMPLE_MS));
    }
}

void gear_estimator_init(void) {
    memset(gear_coeffs, 0, sizeof(gear_coeffs));
    memset(calibration_counts, 0, sizeof(calibration_counts));
    memset(calibration_sums, 0, sizeof(calibration_sums));
    current_input = (GearEstimatorInput){0};
}

void gear_estimator_set_input(float rpm, float speed_kmh) {
    current_input.rpm += (rpm-current_input.rpm) * 0.3f; // Smooth RPM input
    current_input.speed_kmh += (speed_kmh-current_input.speed_kmh) * 0.3f;
    if (current_input.speed_kmh > 0.1f && current_input.rpm > 300.0f) {
        current_input.ratio = current_input.rpm / current_input.speed_kmh;
    } else {
        current_input.ratio = 0.0f;
    }
    ble_module_update_values(current_input.rpm, current_input.speed_kmh);
}

int gear_estimator_get_current_gear(void) {
    if (current_input.ratio < 0.0005f)
        return 0; // Invalid or neutral

    float min_diff = 1e9f;
    int best_gear = 0;

    for (int i = 0; i < MAX_GEARS; ++i) {
        if (gear_coeffs[i] > 0.0f) {
            float diff = fabsf(current_input.ratio - gear_coeffs[i]);
            if (diff < min_diff) {
                min_diff = diff;
                best_gear = i + 1; // gear index is 1-based
            }
        }
    }

    return best_gear;
}

float gear_estimator_get_current_ratio(void) {
    return current_input.ratio;
}

void gear_estimator_get_coefficients(float* out_coeffs, size_t max_len) {
    size_t count = (max_len < MAX_GEARS) ? max_len : MAX_GEARS;
    memcpy(out_coeffs, gear_coeffs, count * sizeof(float));
}

void gear_estimator_set_coefficients(const float* in_coeffs, size_t len) {
    size_t count = (len < MAX_GEARS) ? len : MAX_GEARS;
    memcpy(gear_coeffs, in_coeffs, count * sizeof(float));
}

void gear_estimator_calibrate(int gear) {
    if (gear < 1 || gear > MAX_GEARS || current_input.ratio < 0.01f)
        return;

    int idx = gear - 1;
    calibration_sums[idx] += current_input.ratio;
    calibration_counts[idx] += 1;
    gear_coeffs[idx] = calibration_sums[idx] / calibration_counts[idx];
}

void gear_speed_emulator_init(unsigned char mode) {
    emulator_mode = mode <= 2 ? mode : 0;
    emulator_rpm = 0.0f;
    emulator_speed = 0.0f;
    emulator_gear_index = DYNAMIC_INITIAL_GEAR_INDEX;

    if (emulator_mode == 2) {
        xTaskCreatePinnedToCore(gear_speed_emulator_task, "rpm_speed_task", 2048,
                                NULL, 6, NULL, 1);
    }
}

void gear_speed_emulator_set_rpm(float rpm) {
    emulator_rpm = rpm;
    if (emulator_mode == 1) {
        emulator_speed = rpm > 0.0f ? rpm * gear_coeffs[STATIC_SPEED_GEAR_INDEX] : 0.0f;
        emulator_gear_index = STATIC_SPEED_GEAR_INDEX;
    }
}

float gear_speed_emulator_get_speed(void) {
    return emulator_mode == 0 ? 0.0f : emulator_speed;
}

int gear_speed_emulator_get_gear(void) {
    return emulator_gear_index + 1;
}
