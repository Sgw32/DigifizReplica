#ifndef DIGIFIZ_SPEED_FILTER_H
#define DIGIFIZ_SPEED_FILTER_H

#include <stdint.h>

#define SPEED_FILTER_DEFAULT_K 100u
#define SPEED_FILTER_MAX_K 1000u

/* Match the A2560/RPM LPF coefficient scale: larger K responds faster.
 * At 32 Hz, K=100 reaches 95% of a step in about 0.9 seconds (tau ~0.3 s).
 * Older ESP32 configurations saved this previously unused parameter as zero;
 * treat zero as the default so upgrading cannot freeze the speed display.
 */
static inline float speed_filter_update(float filtered, float speed, uint32_t coefficient)
{
    if (coefficient == 0u) {
        coefficient = SPEED_FILTER_DEFAULT_K;
    } else if (coefficient > SPEED_FILTER_MAX_K) {
        coefficient = SPEED_FILTER_MAX_K;
    }
    return filtered + (speed - filtered) * ((float)coefficient / SPEED_FILTER_MAX_K);
}

#endif
