#ifndef GEAR_ESTIMATOR_H
#define GEAR_ESTIMATOR_H

#include <stddef.h>


//6 gears + reverse
#define MAX_GEARS 7

#ifdef __cplusplus
extern "C" {
#endif

void gear_estimator_init(void);

// Set current RPM and speed (in KMH)
void gear_estimator_set_input(float rpm, float speed_kmh);

// Estimate current gear based on input
int gear_estimator_get_current_gear(void);

// Get gear coefficient table
void gear_estimator_get_coefficients(float* out_coeffs, size_t max_len);

// Set gear coefficient table manually
void gear_estimator_set_coefficients(const float* in_coeffs, size_t len);

// Add calibration data for known gear
void gear_estimator_calibrate(int gear);  // gear is 1-based (1 to 6)

// Return the current calculated ratio (rpm/speed)
float gear_estimator_get_current_ratio(void);

#ifdef __cplusplus
}
#endif

#endif // GEAR_ESTIMATOR_H
