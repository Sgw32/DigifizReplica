#ifndef GEAR_ESTIMATOR_H
#define GEAR_ESTIMATOR_H

/**
 * @file gear_estimator.h
 * @brief Gear estimation API based on RPM-to-speed ratio.
 */

#include <stddef.h>


//6 gears + reverse
#define MAX_GEARS 7

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize estimator state and default coefficients.
 */
void gear_estimator_init(void);

/**
 * @brief Provide latest engine speed and vehicle speed sample.
 *
 * @param rpm Current engine speed.
 * @param speed_kmh Current vehicle speed in km/h.
 */
void gear_estimator_set_input(float rpm, float speed_kmh);

/**
 * @brief Estimate currently engaged gear.
 *
 * @return int Estimated gear number (implementation-defined for neutral/reverse).
 */
int gear_estimator_get_current_gear(void);

/**
 * @brief Copy active gear coefficient table into caller buffer.
 *
 * @param out_coeffs Destination coefficient array.
 * @param max_len Maximum number of coefficients to copy.
 */
void gear_estimator_get_coefficients(float* out_coeffs, size_t max_len);

/**
 * @brief Replace coefficient table with caller-provided values.
 *
 * @param in_coeffs Source coefficient array.
 * @param len Number of coefficients in @p in_coeffs.
 */
void gear_estimator_set_coefficients(const float* in_coeffs, size_t len);

/**
 * @brief Add calibration sample for a known engaged gear.
 *
 * @param gear Gear number (1-based, typically 1..6).
 */
void gear_estimator_calibrate(int gear);  // gear is 1-based (1 to 6)

/**
 * @brief Get current instantaneous RPM/speed ratio.
 *
 * @return float Current ratio used for estimation.
 */
float gear_estimator_get_current_ratio(void);

#ifdef __cplusplus
}
#endif

#endif // GEAR_ESTIMATOR_H
