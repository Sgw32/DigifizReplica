#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

/**
 * @file led_effects.h
 * @brief Color and animation effect helpers for addressable LEDs.
 */

#include <stdint.h>
#include <stddef.h>

/**
 * @brief RGB color triplet.
 */
typedef struct {
    /** @brief Red channel intensity (0..255). */
    uint8_t r;
    /** @brief Green channel intensity (0..255). */
    uint8_t g;
    /** @brief Blue channel intensity (0..255). */
    uint8_t b;
} led_rgb_t;

/**
 * @brief HSV color triplet.
 */
typedef struct {
    /** @brief Hue component (0..255). */
    uint8_t h;
    /** @brief Saturation component (0..255). */
    uint8_t s;
    /** @brief Value/brightness component (0..255). */
    uint8_t v;
} led_hsv_t;

/**
 * @brief Available procedural LED effects.
 */
typedef enum {
    /** @brief Static color output. */
    LED_EFFECT_NONE = 0,
    /** @brief Cyclic hue/gradient animation. */
    LED_EFFECT_CYCLE,
    /** @brief Flame-like procedural animation. */
    LED_EFFECT_FIRE,
    /** @brief Magic sparkle/fantasy animation preset. */
    LED_EFFECT_MAGIC,
    /** @brief Plasma field animation. */
    LED_EFFECT_PLASMA,
    /** @brief Cloud-style smooth evolving pattern. */
    LED_EFFECT_CLOUDS,
    /** @brief Slowly fading trail effect. */
    LED_EFFECT_AFTERGLOW,
    /** @brief Dynamic hue-shift effect. */
    LED_EFFECT_COLOR_SHIFT,
    /** @brief Segment-based fire animation. */
    LED_EFFECT_SEGMENT_FIRE,
    /** @brief Rainbow sweep pattern. */
    LED_EFFECT_RAINBOW,
    /** @brief Random glitter particles. */
    LED_EFFECT_GLITTER,
    /** @brief Traveling wave pattern. */
    LED_EFFECT_WAVE,
    /** @brief Breathing brightness effect. */
    LED_EFFECT_BREATHING,
    /** @brief Confetti particle bursts. */
    LED_EFFECT_CONFETTI,
    /** @brief Number of supported effect types. */
    LED_EFFECT_MAX
} led_effect_t;

/**
 * @brief Mutable state used by LED effect renderer.
 */
typedef struct {
    /** @brief Currently selected effect type. */
    led_effect_t effect;
    /** @brief Base hue used by effect algorithms. */
    uint8_t hue;
    /** @brief Base saturation used by effect algorithms. */
    uint8_t saturation;
    /** @brief Base value/brightness used by effect algorithms. */
    uint8_t value;
    /** @brief Animation phase accumulator. */
    float phase;
} led_effect_state_t;

/**
 * @brief Initialize LED effect runtime state.
 *
 * @param state Effect state object to initialize.
 */
void led_effect_init(led_effect_state_t *state);

/**
 * @brief Compute RGB color for an LED index for the active effect.
 *
 * @param state Effect state object.
 * @param index Zero-based LED index.
 * @return led_rgb_t Calculated RGB color.
 */
led_rgb_t led_effect_compute(led_effect_state_t *state, size_t index);

/**
 * @brief Advance effect phase by elapsed time delta.
 *
 * @param state Effect state object.
 * @param delta Time step in seconds (or implementation-defined units).
 */
void led_effect_step(led_effect_state_t *state, float delta);

#endif // LED_EFFECTS_H
