#include "block_color_alert.h"
#include "block_color_alert_params.h"
#include "xparam.h"
#include <assert.h>
#include <stdio.h>

typedef struct { NEXT_BLOCK_COLOR_ALERT_PARAMS(DECLARE_PARAM) } alert_params_t;
static const alert_params_t defaults = { NEXT_BLOCK_COLOR_ALERT_PARAMS(DEFINE_PARAM) };

static bool same_color(led_rgb_t a, led_rgb_t b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

static void test_thresholds(void)
{
    const float thresholds[] = {6000, 120, 10, 110, 130};
    const float bands[] = {200, 5, 2, 5, 5};
    for (unsigned block = 0; block < BLOCK_COUNT; ++block) {
        block_color_config_t config = {
            .mode = BLOCK_COLOR_BLINK, .threshold = thresholds[block],
            .hysteresis = bands[block], .below = block == BLOCK_FUEL,
        };
        bool low = config.below;
        assert(!block_color_update(false, config, config.threshold, true));
        float trigger = config.threshold + (low ? -1 : 1);
        bool active = block_color_update(false, config, trigger, true);
        assert(active);
        assert(block_color_update(active, config, config.threshold, true));
        float recovery = config.threshold + (low ? config.hysteresis : -config.hysteresis);
        assert(block_color_update(active, config, recovery + (low ? -0.1f : 0.1f), true));
        assert(!block_color_update(active, config, recovery, true));
        // Invalid sensors, hidden oil pages/gear displays and disabling a mode
        // clear latches; re-enabling inside the recovery band cannot relatch.
        assert(!block_color_update(active, config, trigger, false));
        assert(!block_color_update(active, config, NAN, true));
        assert(!block_color_update(active, config, INFINITY, true));
        config.mode = BLOCK_COLOR_NONE;
        assert(!block_color_update(active, config, trigger, true));
        config.mode = BLOCK_COLOR_CONSTANT;
        assert(!block_color_update(false, config, recovery, true));
    }
    puts("PASS: all five thresholds, hysteresis boundaries, modes and invalid/hidden inputs");
}

static void test_clock(void)
{
    block_color_clock_t clock = {0};
    assert(block_color_phase(&clock, 100, 1000));
    assert(block_color_phase(&clock, 599, 1000));
    assert(!block_color_phase(&clock, 600, 1000));
    assert(!block_color_phase(&clock, 1099, 1000));
    assert(block_color_phase(&clock, 1100, 1000));
    assert(!block_color_phase(&clock, 4700, 1000)); // Missed frames don't shift phase.
    assert(block_color_phase(&clock, 4701, 2000)); // Live period change resets all together.
    assert(!block_color_phase(&clock, 5701, 2000));
    assert(block_color_phase(&clock, 5702, 0)); // Invalid saved period safely defaults.
    assert(clock.period_ms == 1000);
    clock = (block_color_clock_t){0};
    assert(block_color_phase(&clock, UINT32_MAX - 250, 1000));
    assert(!block_color_phase(&clock, 249, 1000));
    assert(block_color_phase(&clock, 749, 1000));
    puts("PASS: synchronized full-cycle timing, missed frames, live settings and millis rollover");
}

static void test_rendering(void)
{
    assert(BLOCK_COLOR_CONSTANT == 1); // Saved speed enables stay constant, not blinking.
    block_color_clock_t clock = {0};
    bool phase = block_color_phase(&clock, 0, 1000);
    for (unsigned block = 0; block < BLOCK_COUNT; ++block) {
        led_rgb_t selected = {20 + block, 64, 128};
        block_color_config_t config = {.mode = BLOCK_COLOR_BLINK, .color = {255, 5, 7}};
        assert(same_color(block_color_render(selected, config, true, phase), config.color));
        assert(same_color(block_color_render(selected, config, true, false), selected));
        assert(same_color(block_color_render(selected, config, false, true), selected));
        config.mode = BLOCK_COLOR_CONSTANT;
        assert(same_color(block_color_render(selected, config, true, false), config.color));
        config.color = (led_rgb_t){0, 0, 255};
        assert(same_color(block_color_render(selected, config, true, true), config.color));
        config.mode = BLOCK_COLOR_NONE;
        assert(same_color(block_color_render(selected, config, false, true), selected));
    }
    puts("PASS: base/theme colors restored, constant color and live RGB changes share one phase");
}

static void test_led_ranges(void)
{
    unsigned counts[BLOCK_COUNT] = {0};
    for (unsigned led = 0; led < 303; ++led) {
        int block = block_color_section(led);
        if (block >= 0) ++counts[block];
    }
    assert(counts[BLOCK_RPM] == 63);
    assert(counts[BLOCK_SPEED] == 43);
    assert(counts[BLOCK_FUEL] == 21);
    assert(counts[BLOCK_COOLANT] == 15);
    assert(counts[BLOCK_OIL] == 31);
    assert(block_color_section(227) == BLOCK_SPEED); // Hundreds digit, missed by old alert.
    assert(block_color_section(269) == BLOCK_SPEED);
    assert(block_color_section(19) == -1); // Oil-pressure icon is not oil temperature.
    assert(block_color_section(150) == -1); // Clock is not MFA oil temperature.
    assert(block_color_section(270) == -1); // Upper/backlight unaffected.
    puts("PASS: physical block coverage and unrelated icons/clock/backlights excluded");
}

static void test_defaults_and_extreme_settings(void)
{
    assert(defaults.blockAlertBlinkPeriodMs.value == 1000);
    assert(defaults.rpmColorChangeMode.value == 0);
    assert(defaults.fuelColorChangeMode.value == 0);
    assert(defaults.coolantColorChangeMode.value == 0);
    assert(defaults.oilColorChangeMode.value == 0);
    assert(defaults.rpmColorChangeThreshold.value == 6000);
    assert(defaults.fuelColorChangeThreshold.value == 10);
    assert(defaults.coolantColorChangeThreshold.value == 110);
    assert(defaults.oilColorChangeThreshold.value == 130);
    block_color_config_t config = {.mode = BLOCK_COLOR_CONSTANT, .threshold = 100, .hysteresis = 200};
    assert(!block_color_update(true, config, 0, true)); // Recovery cannot require a negative RPM.
    config.mode = 255;
    assert(!block_color_update(true, config, 200, true));
    puts("PASS: actual parameter defaults, excessive hysteresis and invalid saved modes");
}

int main(void)
{
    test_thresholds();
    test_clock();
    test_rendering();
    test_led_ranges();
    test_defaults_and_extreme_settings();
    return 0;
}
