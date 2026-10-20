#ifndef DIGIFIZ_FREQUENCY_WINDOW_H
#define DIGIFIZ_FREQUENCY_WINDOW_H

#include <stdint.h>

typedef struct {
    uint64_t time_us;
    uint32_t count;
} frequency_snapshot_t;

typedef struct {
    frequency_snapshot_t *samples;
    unsigned capacity;
    unsigned first;
    unsigned size;
    uint64_t window_us;
} frequency_window_t;

/* Cumulative hardware counts: never clear between samples. Unsigned
 * subtraction also handles a wrap of the driver's accumulated count. */
static inline double frequency_window_push(frequency_window_t *window,
                                          uint64_t time_us, uint32_t count)
{
    if (window->size == window->capacity) {
        window->first = (window->first + 1u) % window->capacity;
        --window->size;
    }
    unsigned next = (window->first + window->size) % window->capacity;
    window->samples[next] = (frequency_snapshot_t){time_us, count};
    ++window->size;
    // Retain the closest sample at or before the requested window boundary.
    while (window->size > 2u) {
        unsigned second = (window->first + 1u) % window->capacity;
        if (time_us - window->samples[second].time_us < window->window_us) {
            break;
        }
        window->first = second;
        --window->size;
    }
    frequency_snapshot_t oldest = window->samples[window->first];
    uint64_t elapsed = time_us - oldest.time_us;
    return elapsed ? (double)(uint32_t)(count - oldest.count) * 1000000.0 / elapsed : 0.0;
}

#endif
