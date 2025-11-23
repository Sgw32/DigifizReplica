#include "led_effects.h"
#include <math.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_random.h>

static inline uint32_t rand32(void)
{
    return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
}

static led_rgb_t hsv2rgb(led_hsv_t hsv)
{
    float h = (float)hsv.h * 360.0f / 255.0f;
    float s = hsv.s / 255.0f;
    float v = hsv.v / 255.0f;

    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - c;

    float rf = 0, gf = 0, bf = 0;

    if (h < 60) {
        rf = c; gf = x; bf = 0;
    } else if (h < 120) {
        rf = x; gf = c; bf = 0;
    } else if (h < 180) {
        rf = 0; gf = c; bf = x;
    } else if (h < 240) {
        rf = 0; gf = x; bf = c;
    } else if (h < 300) {
        rf = x; gf = 0; bf = c;
    } else {
        rf = c; gf = 0; bf = x;
    }

    led_rgb_t rgb;
    rgb.r = (uint8_t)((rf + m) * 255.0f);
    rgb.g = (uint8_t)((gf + m) * 255.0f);
    rgb.b = (uint8_t)((bf + m) * 255.0f);
    return rgb;
}

void led_effect_init(led_effect_state_t *state)
{
    if (!state) return;
    state->phase = 0.0f;
    srand((unsigned)esp_random());
}

led_rgb_t led_effect_compute(led_effect_state_t *state, size_t index)
{
    led_hsv_t h;
    h.h = state->hue;
    h.s = state->saturation;
    h.v = state->value;

    switch (state->effect) {
    case LED_EFFECT_CYCLE:
        h.h += (uint8_t)(sinf(state->phase / 20.0f) * 5 + index * 4);
        break;
    case LED_EFFECT_FIRE: {
        float flicker = sinf(state->phase * 0.03f + index * 0.3f) * (rand32() & 15) + 15;
        if (h.v > (uint8_t)flicker)
            h.v -= (uint8_t)flicker;
        h.h += (uint8_t)(sinf(state->phase * 0.2f + index * 3) * 10.0f);
        break;
    }
    case LED_EFFECT_MAGIC:
        h.h += (uint8_t)state->phase + index * 5;
        break;
    case LED_EFFECT_PLASMA:
        h.h += (uint8_t)(sinf(state->phase * 0.1f + index * 0.5f) * 40.0f) +
               (uint8_t)(cosf(state->phase * 0.2f + index * 0.3f) * 40.0f);
        break;
    case LED_EFFECT_CLOUDS:
        h.h += (uint8_t)(sinf(state->phase * 0.05f + index * 0.3f) * 20.0f);
        h.v -= (uint8_t)((cosf(state->phase * 0.07f + index * 0.2f) + 1.0f) * 10.0f);
        break;
    case LED_EFFECT_AFTERGLOW:
        h.h += (uint8_t)state->phase + index * 10;
        h.v = (uint8_t)((sinf(state->phase * 0.1f + index * 0.4f) + 1.0f) * state->value / 2);
        break;
    case LED_EFFECT_COLOR_SHIFT:
        h.h += (uint8_t)(state->phase * 2.0f);
        break;
    case LED_EFFECT_SEGMENT_FIRE: {
        float fire = sinf(state->phase * 0.03f + index) * (rand32() & 15);
        if (h.v > (uint8_t)fire) h.v -= (uint8_t)fire;
        h.h += (uint8_t)(rand32() & 5);
        break;
    }
    case LED_EFFECT_RAINBOW:
        h.h = (uint8_t)(index * 256 / 283) + (uint8_t)state->phase;
        break;
    case LED_EFFECT_GLITTER:
        if ((rand32() & 0x1F) == 0) h.v = state->value;
        break;
    case LED_EFFECT_WAVE:
        h.v = (uint8_t)((sinf(state->phase + index * 0.4f) + 1.0f) * state->value / 2);
        break;
    case LED_EFFECT_BREATHING:
        h.v = (uint8_t)((sinf(state->phase * 0.05f) + 1.0f) * state->value / 2);
        break;
    case LED_EFFECT_CONFETTI:
        h.h += (uint8_t)(state->phase * 5.0f + index * 20);
        if ((rand32() & 0x7) == 0)
            h.v = state->value;
        else
            h.v = 0;
        break;
    case LED_EFFECT_NONE:
    default:
        break;
    }

    return hsv2rgb(h);
}

void led_effect_step(led_effect_state_t *state, float delta)
{
    if (!state) return;
    state->phase += delta;
}

