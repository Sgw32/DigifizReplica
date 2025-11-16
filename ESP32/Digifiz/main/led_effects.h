#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_rgb_t;

typedef struct {
    uint8_t h;
    uint8_t s;
    uint8_t v;
} led_hsv_t;

typedef enum {
    LED_EFFECT_NONE = 0,
    LED_EFFECT_CYCLE,
    LED_EFFECT_FIRE,
    LED_EFFECT_MAGIC,
    LED_EFFECT_PLASMA,
    LED_EFFECT_CLOUDS,
    LED_EFFECT_AFTERGLOW,
    LED_EFFECT_COLOR_SHIFT,
    LED_EFFECT_SEGMENT_FIRE,
    LED_EFFECT_RAINBOW,
    LED_EFFECT_GLITTER,
    LED_EFFECT_WAVE,
    LED_EFFECT_BREATHING,
    LED_EFFECT_CONFETTI,
    LED_EFFECT_MAX
} led_effect_t;

typedef struct {
    led_effect_t effect;
    uint8_t hue;
    uint8_t saturation;
    uint8_t value;
    float phase;
} led_effect_state_t;

void led_effect_init(led_effect_state_t *state);
led_rgb_t led_effect_compute(led_effect_state_t *state, size_t index);
void led_effect_step(led_effect_state_t *state, float delta);

#endif // LED_EFFECTS_H
