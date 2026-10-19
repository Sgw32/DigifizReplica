#include "speed_filter.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void test_default_response(void)
{
    float speed = speed_filter_update(0.0f, 100.0f, SPEED_FILTER_DEFAULT_K);
    assert(fabsf(speed - 10.0f) < 0.0001f);
    speed = speed_filter_update(speed, 100.0f, SPEED_FILTER_DEFAULT_K);
    assert(fabsf(speed - 19.0f) < 0.0001f);

    unsigned cycles_to_95_percent = 0;
    speed = 0.0f;
    for (unsigned i = 1; i <= 32; ++i) {
        speed = speed_filter_update(speed, 100.0f, SPEED_FILTER_DEFAULT_K);
        if (cycles_to_95_percent == 0 && speed >= 95.0f) {
            cycles_to_95_percent = i;
        }
    }
    assert(cycles_to_95_percent == 29);
    assert(speed > 96.5f && speed < 96.7f);
    printf("PASS: default K=100 reaches 95%% in %.3f s at 32 Hz\n",
           cycles_to_95_percent / 32.0);
}

static void test_stop_response(void)
{
    float speed = 100.0f;
    for (unsigned i = 0; i < 32; ++i) {
        float previous = speed;
        speed = speed_filter_update(speed, 0.0f, SPEED_FILTER_DEFAULT_K);
        assert(speed >= 0.0f && speed < previous);
    }
    assert(speed > 3.3f && speed < 3.5f);
    for (unsigned i = 0; i < 128; ++i) {
        speed = speed_filter_update(speed, 0.0f, SPEED_FILTER_DEFAULT_K);
    }
    assert(speed < 0.001f);
    puts("PASS: stop/no-signal decays toward zero with the same response");
}

static void test_coefficient_range(void)
{
    // Saved zero was the old ESP32 default; it must not hold speed at zero.
    assert(speed_filter_update(0.0f, 100.0f, 0) ==
           speed_filter_update(0.0f, 100.0f, SPEED_FILTER_DEFAULT_K));
    assert(speed_filter_update(0.0f, 100.0f, 1) > 0.0f);
    assert(speed_filter_update(0.0f, 100.0f, 500) == 50.0f);
    assert(speed_filter_update(0.0f, 100.0f, 1000) == 100.0f);
    assert(speed_filter_update(80.0f, 0.0f, 1000) == 0.0f);
    assert(speed_filter_update(0.0f, 100.0f, 65535) == 100.0f);
    assert(speed_filter_update(80.0f, 0.0f, UINT32_MAX) == 0.0f);
    puts("PASS: legacy zero uses default; K=1..1000 works; oversized K cannot overshoot");
}

static void test_live_coefficient_changes(void)
{
    float speed = speed_filter_update(0.0f, 100.0f, 100);
    speed = speed_filter_update(speed, 100.0f, 500);
    assert(speed == 55.0f);
    speed = speed_filter_update(speed, 100.0f, 1000);
    assert(speed == 100.0f);
    speed = speed_filter_update(speed, 0.0f, 100);
    assert(speed == 90.0f);
    puts("PASS: live coefficient changes take effect without resetting filter state");
}

static void test_noise_attenuation(void)
{
    float speed = 80.0f;
    double sum = 0.0;
    double squared_error = 0.0;
    for (unsigned i = 0; i < 256; ++i) {
        float raw_speed = i % 2 ? 100.0f : 60.0f;
        speed = speed_filter_update(speed, raw_speed, SPEED_FILTER_DEFAULT_K);
        if (i >= 128) {
            sum += speed;
            squared_error += (speed - 80.0f) * (speed - 80.0f);
        }
    }
    assert(fabs(sum / 128.0 - 80.0) < 0.01);
    assert(sqrt(squared_error / 128.0) < 1.1);
    puts("PASS: alternating +/-20 speed noise is reduced to about +/-1 around its mean");
}

int main(void)
{
    test_default_response();
    test_stop_response();
    test_coefficient_range();
    test_live_coefficient_changes();
    test_noise_attenuation();
    puts("All speed LPF tests passed.");
    return 0;
}
