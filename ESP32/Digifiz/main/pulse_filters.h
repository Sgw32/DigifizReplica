#ifndef DIGIFIZ_PULSE_FILTERS_H
#define DIGIFIZ_PULSE_FILTERS_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Pure signal-processing helpers shared by firmware and host waveform tests. */
#define PULSE_WINDOW_SIZE 8u
#define PULSE_TIMEOUT_US 1000000u
#define RPM_RC_SAMPLE_US 25u
#define RPM_RC_TAU_US 400u
#define RPM_RC_ONE 65536
/* round((1 - exp(-25 / 400)) * 65536). No floating point in the ISR. */
#define RPM_RC_ALPHA 3971
#define RPM_RC_LOW 29491  /* 45% */
#define RPM_RC_HIGH 36045 /* 55% */

typedef struct {
    int32_t voltage;
    bool high;
    uint64_t last_crossing_us;
    bool have_crossing;
    uint32_t periods[PULSE_WINDOW_SIZE];
    uint64_t sum_us;
    unsigned count;
    unsigned index;
} rpm_rc_filter_t;

static inline void rpm_rc_init(rpm_rc_filter_t *filter, bool high)
{
    memset(filter, 0, sizeof(*filter));
    filter->voltage = high ? RPM_RC_ONE : 0;
    filter->high = high;
}

/* Called once per 25 us sample. Returns a rolling mean period on each
 * rising threshold crossing; startup uses only actual periods, up to eight.
 * Thresholds suppress chatter without changing the RC time constant. */
static inline bool rpm_rc_sample(rpm_rc_filter_t *filter, bool input_high,
                                 uint64_t now_us, uint32_t *period_us)
{
    int32_t target = input_high ? RPM_RC_ONE : 0;
    filter->voltage += ((target - filter->voltage) * RPM_RC_ALPHA) / RPM_RC_ONE;
    if (filter->high) {
        if (filter->voltage <= RPM_RC_LOW) {
            filter->high = false;
        }
        return false;
    }
    if (filter->voltage < RPM_RC_HIGH) {
        return false;
    }
    filter->high = true;
    uint64_t elapsed = now_us - filter->last_crossing_us;
    filter->last_crossing_us = now_us;
    if (!filter->have_crossing || elapsed >= PULSE_TIMEOUT_US) {
        filter->have_crossing = true;
        filter->count = filter->index = 0;
        filter->sum_us = 0;
        return false;
    }
    if (elapsed == 0) {
        return false;
    }
    if (filter->count == PULSE_WINDOW_SIZE) {
        filter->sum_us -= filter->periods[filter->index];
    } else {
        ++filter->count;
    }
    filter->periods[filter->index] = (uint32_t)elapsed;
    filter->sum_us += elapsed;
    filter->index = (filter->index + 1u) % PULSE_WINDOW_SIZE;
    *period_us = (uint32_t)(filter->sum_us / filter->count);
    return true;
}

typedef struct {
    bool high;
    bool candidate;
    bool have_rising;
    uint64_t edge_us;
    uint64_t rising_us;
    uint64_t valid_rising_us;
} speed_pulse_filter_t;

static inline void speed_pulse_init(speed_pulse_filter_t *filter, bool high,
                                   uint64_t now_us)
{
    memset(filter, 0, sizeof(*filter));
    filter->high = high;
    filter->edge_us = now_us;
}

/* Qualify both low and high widths. Rejected glitches never replace the
 * accepted rising timestamp, so they cannot shorten the next valid period. */
static inline bool speed_pulse_edge(speed_pulse_filter_t *filter, bool high,
                                   uint64_t now_us, uint32_t min_width_us,
                                   uint32_t *period_us)
{
    if (high == filter->high) {
        return false;
    }
    uint64_t width = now_us - filter->edge_us;
    filter->edge_us = now_us;
    filter->high = high;
    if (high) {
        filter->candidate = width >= min_width_us;
        filter->rising_us = now_us;
        return false;
    }
    if (!filter->candidate || width < min_width_us) {
        filter->candidate = false;
        return false;
    }
    filter->candidate = false;
    uint64_t elapsed = filter->rising_us - filter->valid_rising_us;
    filter->valid_rising_us = filter->rising_us;
    if (!filter->have_rising || elapsed >= PULSE_TIMEOUT_US) {
        filter->have_rising = true;
        return false;
    }
    if (elapsed < min_width_us) {
        return false;
    }
    *period_us = (uint32_t)elapsed;
    return true;
}

typedef struct {
    uint32_t periods[PULSE_WINDOW_SIZE];
    unsigned count;
    unsigned index;
} speed_median_filter_t;

static inline uint32_t speed_median_period(speed_median_filter_t *filter,
                                         uint32_t period_us)
{
    filter->periods[filter->index] = period_us;
    filter->index = (filter->index + 1u) % PULSE_WINDOW_SIZE;
    if (filter->count < PULSE_WINDOW_SIZE) {
        ++filter->count;
    }
    uint32_t sorted[PULSE_WINDOW_SIZE];
    for (unsigned i = 0; i < filter->count; ++i) {
        sorted[i] = filter->periods[i];
    }
    for (unsigned i = 1; i < filter->count; ++i) {
        uint32_t key = sorted[i];
        unsigned j = i;
        while (j > 0 && sorted[j - 1] > key) {
            sorted[j] = sorted[j - 1];
            --j;
        }
        sorted[j] = key;
    }
    return sorted[filter->count / 2u];
}

#endif
