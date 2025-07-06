#include "gear_estimator.h"
#include "ble_module.h"
#include <string.h>
#include <math.h>

typedef struct {
    float rpm;
    float speed_kmh;
    float ratio; // computed as rpm / speed
} GearEstimatorInput;

static float gear_coeffs[MAX_GEARS] = {0}; // estimated coefficients
static int calibration_counts[MAX_GEARS] = {0};
static float calibration_sums[MAX_GEARS] = {0};

static GearEstimatorInput current_input = {0};

void gear_estimator_init(void) {
    memset(gear_coeffs, 0, sizeof(gear_coeffs));
    memset(calibration_counts, 0, sizeof(calibration_counts));
    memset(calibration_sums, 0, sizeof(calibration_sums));
    current_input = (GearEstimatorInput){0};
}

void gear_estimator_set_input(float rpm, float speed_kmh) {
    current_input.rpm += (rpm-current_input.rpm) * 0.3f; // Smooth RPM input
    current_input.speed_kmh += (speed_kmh-current_input.speed_kmh) * 0.3f;
    current_input.ratio = (rpm > 300.0f) ? (speed_kmh / rpm) : 0.0f;
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
