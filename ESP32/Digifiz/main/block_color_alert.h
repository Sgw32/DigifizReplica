#ifndef BLOCK_COLOR_ALERT_H
#define BLOCK_COLOR_ALERT_H

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "led_effects.h"

/* Keep value 1 compatible with the original speed-color enable parameter. */
enum { BLOCK_COLOR_NONE = 0, BLOCK_COLOR_CONSTANT = 1, BLOCK_COLOR_BLINK = 2 };
enum { BLOCK_RPM, BLOCK_SPEED, BLOCK_FUEL, BLOCK_COOLANT, BLOCK_OIL, BLOCK_COUNT };

typedef struct {
    uint8_t mode;
    float threshold;
    float hysteresis;
    bool below;
    led_rgb_t color;
} block_color_config_t;

typedef struct {
    uint32_t last_ms;
    uint32_t phase_ms;
    uint32_t period_ms;
    bool initialized;
} block_color_clock_t;

/* One clock for all blocks, including alerts which become active later.
 * period_ms is a FULL cycle; each color occupies half of that cycle. */
static inline bool block_color_phase(block_color_clock_t *clock, uint32_t now, uint32_t period_ms)
{
    if (period_ms < 100 || period_ms > 10000) period_ms = 1000;
    if (!clock->initialized || clock->period_ms != period_ms) {
        clock->initialized = true;
        clock->phase_ms = 0;
        clock->period_ms = period_ms;
    } else {
        uint32_t elapsed = now - clock->last_ms; // Works across millis() rollover.
        clock->phase_ms = ((uint64_t)clock->phase_ms + elapsed) % period_ms;
    }
    clock->last_ms = now;
    return clock->phase_ms < period_ms / 2;
}

static inline bool block_color_update(bool active, block_color_config_t config, float value, bool valid)
{
    if (!valid || !isfinite(value) || config.mode == BLOCK_COLOR_NONE || config.mode > BLOCK_COLOR_BLINK)
        return false;
    if (config.below)
        return active ? value < config.threshold + config.hysteresis : value < config.threshold;
    float release = fmaxf(0, config.threshold - config.hysteresis);
    return active ? value > release : value > config.threshold;
}

/* Physical bit/LED offsets in DigifizNextDisplay, NOT color-scheme indexes.
 * Include all six speed digits (the old speed override missed the first).
 * Oil applies to MFA digits/dots only, never to the oil-pressure icon. */
static inline int block_color_section(unsigned led)
{
    if (led <= 14) return BLOCK_COOLANT;
    if (led >= 32 && led <= 94) return BLOCK_RPM;
    if (led >= 175 && led <= 205) return BLOCK_OIL;
    if (led >= 206 && led <= 226) return BLOCK_FUEL;
    if (led >= 227 && led <= 269) return BLOCK_SPEED;
    return -1;
}

static inline led_rgb_t block_color_render(led_rgb_t base, block_color_config_t config,
                                           bool active, bool alert_phase)
{
    if (active && (config.mode == BLOCK_COLOR_CONSTANT ||
                   (config.mode == BLOCK_COLOR_BLINK && alert_phase))) return config.color;
    return base;
}

#endif
