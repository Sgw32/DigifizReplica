#include "pulse_filters.h"
#include "frequency_window.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void test_rc_response(void)
{
    rpm_rc_filter_t filter;
    rpm_rc_init(&filter, false);
    uint32_t period;
    for (unsigned i = 1; i <= 64; ++i) {
        rpm_rc_sample(&filter, true, i * RPM_RC_SAMPLE_US, &period);
        double expected = 1.0 - exp(-(double)(i * RPM_RC_SAMPLE_US) / RPM_RC_TAU_US);
        assert(fabs((double)filter.voltage / RPM_RC_ONE - expected) < 0.001);
    }
    assert(filter.high);
    for (unsigned i = 1; i <= 64; ++i) {
        rpm_rc_sample(&filter, false, (64u + i) * RPM_RC_SAMPLE_US, &period);
    }
    assert(!filter.high);
    puts("PASS: exponential RC response, 400 us time constant and thresholds");
}

static void test_rpm_waveform(unsigned cycle_us, unsigned width_us, bool inverted)
{
    rpm_rc_filter_t filter;
    rpm_rc_init(&filter, !inverted);
    uint32_t period = 0;
    unsigned updates = 0;
    for (uint64_t t = 0; t < (uint64_t)cycle_us * 24u; t += RPM_RC_SAMPLE_US) {
        unsigned phase = t % cycle_us;
        // Representative blue trace: mostly high, periodic low pulses and
        // extra 50 us low spikes well away from the true pulse.
        bool low = phase < width_us || (phase >= cycle_us / 2u && phase < cycle_us / 2u + 50u);
        bool level = inverted ? low : !low;
        if (rpm_rc_sample(&filter, level, t, &period)) {
            ++updates;
            assert(fabs((double)period - cycle_us) <= 2 * RPM_RC_SAMPLE_US);
        }
    }
    assert(updates >= 22 && updates <= 23);
    assert(filter.count == PULSE_WINDOW_SIZE);
    double hz = 1000000.0 / period;
    printf("PASS: %s input, %.3f Hz (%.1f RPM at 2 pulses/rev), %u updates\n",
           inverted ? "positive" : "blue-trace negative", hz, hz * 30.0, updates);
}

static void test_rpm_glitches_and_restart(void)
{
    rpm_rc_filter_t filter;
    uint32_t period;
    rpm_rc_init(&filter, true);
    for (uint64_t t = 0; t < 500000; t += RPM_RC_SAMPLE_US) {
        assert(!rpm_rc_sample(&filter, t % 5000 >= 100, t, &period));
    }
    assert(!filter.have_crossing);
    // Acquire one frequency, then stop long enough to expire the old window.
    unsigned updates = 0;
    for (uint64_t t = 500000; t < 700000; t += RPM_RC_SAMPLE_US) {
        if (rpm_rc_sample(&filter, t % 10000 >= 1000, t, &period)) {
            ++updates;
        }
    }
    assert(updates >= 18 && period == 10000);
    for (uint64_t t = 700000; t < 1900000; t += RPM_RC_SAMPLE_US) {
        assert(!rpm_rc_sample(&filter, true, t, &period));
    }
    updates = 0;
    for (uint64_t t = 1900000; t < 2000000; t += RPM_RC_SAMPLE_US) {
        if (rpm_rc_sample(&filter, t % 20000 >= 1000, t, &period)) {
            ++updates;
            assert(period == 20000); // No pre-stop samples in the new average.
        }
    }
    assert(updates == 4);
    puts("PASS: 100 us glitches rejected, stopped input and restart reset");
}

static void test_rpm_rolling_window(void)
{
    rpm_rc_filter_t filter;
    rpm_rc_init(&filter, true);
    uint32_t period;
    uint64_t t = 0;
    for (; t < 200000; t += RPM_RC_SAMPLE_US) {
        rpm_rc_sample(&filter, t % 10000 >= 1000, t, &period);
    }
    assert(period == 10000 && filter.count == 8);
    unsigned updates = 0;
    for (; t < 400000; t += RPM_RC_SAMPLE_US) {
        if (rpm_rc_sample(&filter, t % 20000 >= 1000, t, &period)) {
            ++updates;
            assert(period >= 10000 && period <= 20000);
            if (updates >= 9) {
                assert(period == 20000);
            }
        }
    }
    assert(updates == 10);
    puts("PASS: eight-period rolling window updates on every pulse during RPM changes");
}

static void test_speed_capture(void)
{
    speed_pulse_filter_t filter;
    speed_pulse_init(&filter, false, 0);
    uint32_t period;
    unsigned readings = 0;
    for (unsigned i = 0; i < 50; ++i) {
        uint64_t rise = 1000u + i * 20000u;
        assert(!speed_pulse_edge(&filter, true, rise, 50, &period));
        if (speed_pulse_edge(&filter, false, rise + 5000u, 50, &period)) {
            ++readings;
            assert(period == 20000);
        }
        // Short high glitch during the low portion must not poison timing.
        assert(!speed_pulse_edge(&filter, true, rise + 10000u, 50, &period));
        assert(!speed_pulse_edge(&filter, false, rise + 10010u, 50, &period));
    }
    assert(readings == 49); // Baseline plus EVERY subsequent pulse, including tenth.
    speed_pulse_init(&filter, false, 0);
    assert(!speed_pulse_edge(&filter, true, 1000, 50, &period));
    assert(!speed_pulse_edge(&filter, false, 6000, 50, &period));
    assert(!speed_pulse_edge(&filter, true, 601000, 50, &period));
    assert(speed_pulse_edge(&filter, false, 606000, 50, &period));
    assert(period == 600000); // No eight-pulse startup delay at low speeds.
    assert(!speed_pulse_edge(&filter, true, 2001000, 50, &period));
    assert(!speed_pulse_edge(&filter, false, 2006000, 50, &period));
    assert(!speed_pulse_edge(&filter, true, 2021000, 50, &period));
    assert(speed_pulse_edge(&filter, false, 2026000, 50, &period));
    assert(period == 20000);
    puts("PASS: speed capture, glitch rejection, low speed, restart and no tenth-pulse loss");
}

static void test_speed_low_glitch(void)
{
    speed_pulse_filter_t filter;
    speed_pulse_init(&filter, false, 0);
    uint32_t period;
    assert(!speed_pulse_edge(&filter, true, 1000, 50, &period));
    assert(!speed_pulse_edge(&filter, false, 6000, 50, &period));
    assert(!speed_pulse_edge(&filter, true, 21000, 50, &period));
    assert(speed_pulse_edge(&filter, false, 22000, 50, &period));
    assert(period == 20000);
    assert(!speed_pulse_edge(&filter, true, 22010, 50, &period));
    assert(!speed_pulse_edge(&filter, false, 26000, 50, &period));
    assert(!speed_pulse_edge(&filter, true, 41000, 50, &period));
    assert(speed_pulse_edge(&filter, false, 46000, 50, &period));
    assert(period == 20000);
    puts("PASS: short low glitch produces no extra speed pulse");
}

static void test_speed_median(void)
{
    speed_median_filter_t filter = {0};
    assert(speed_median_period(&filter, 20000) == 20000);
    assert(speed_median_period(&filter, 20000) == 20000);
    assert(speed_median_period(&filter, 100000) == 20000);
    for (unsigned i = 0; i < 8; ++i) {
        speed_median_period(&filter, 10000);
    }
    assert(filter.count == 8);
    assert(speed_median_period(&filter, 10000) == 10000);
    puts("PASS: speed median has no startup zero padding and removes period outliers");
}

static void test_counter_window(void)
{
    frequency_snapshot_t samples[8];
    frequency_window_t window = {.samples = samples, .capacity = 8, .window_us = 1000000};
    // Different update period and window, plus signed/cumulative count wraps.
    uint32_t baseline = UINT32_MAX - 50;
    assert(frequency_window_push(&window, 0, baseline) == 0);
    for (unsigned i = 1; i <= 20; ++i) {
        double hz = frequency_window_push(&window, i * 250000u, baseline + i * 25u);
        assert(fabs(hz - 100.0) < 0.001);
    }
    // Scheduler delay: denominator must be elapsed time, not requested duration.
    assert(fabs(frequency_window_push(&window, 6500000, baseline + 650) - 100.0) < 0.001);
    puts("PASS: cumulative PCNT window, differing period/window, count wrap and scheduler delay");
}

int main(void)
{
    test_rc_response();
    test_rpm_waveform(14600, 1400, false);
    test_rpm_waveform(14600, 1400, true);
    test_rpm_waveform(3750, 600, false); // About 8000 RPM at 2 pulses/revolution.
    test_rpm_waveform(60000, 1400, false);
    test_rpm_glitches_and_restart();
    test_rpm_rolling_window();
    test_speed_capture();
    test_speed_low_glitch();
    test_speed_median();
    test_counter_window();
    puts("All waveform tests passed.");
    return 0;
}
