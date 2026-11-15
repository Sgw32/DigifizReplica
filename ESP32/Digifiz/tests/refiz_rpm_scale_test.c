#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "xparam.h"

#ifdef TEST_NEXT_DEFAULTS
#include "setup_dr_next.h"
#include "params_list.h"
#else
#include "setup_refiz.h"
#include "params_list_refiz.h"
#include "refiz_rpm_scale.h"
#endif

typedef struct { PARAM_LIST(DECLARE_PARAM) } test_params_t;
static const test_params_t defaults = { PARAM_LIST(DEFINE_PARAM) };

#ifndef TEST_NEXT_DEFAULTS
static void test_step_boundaries(void)
{
    assert(refiz_rpm_lit_segments(0, true, 8000) == 1);
    assert(refiz_rpm_lit_segments(99, true, 8000) == 1);
    assert(refiz_rpm_lit_segments(100, true, 8000) == 2);
    assert(refiz_rpm_lit_segments(6999, true, 8000) == 70);
    assert(refiz_rpm_lit_segments(7000, true, 8000) == 71);
    assert(refiz_rpm_lit_segments(0, false, 8000) == 1);
    assert(refiz_rpm_lit_segments(124, false, 8000) == 1);
    assert(refiz_rpm_lit_segments(125, false, 8000) == 2);
    assert(refiz_rpm_lit_segments(7999, false, 8000) == 64);
    assert(refiz_rpm_lit_segments(8000, false, 8000) == 65);
    for (unsigned step = 0; step <= 70; ++step)
        assert(refiz_rpm_lit_segments(step * 100, true, 10000) == step + 1);
    for (unsigned step = 0; step <= 64; ++step)
        assert(refiz_rpm_lit_segments(step * 125, false, 10000) == step + 1);
    puts("PASS: every 100/125 RPM step, 7k/8k endpoints and fixed zero marker");
}

static void test_saturation_and_maximum(void)
{
    assert(refiz_rpm_lit_segments(9000, true, 10000) == 71);
    assert(refiz_rpm_lit_segments(9000, false, 10000) == 65);
    assert(refiz_rpm_lit_segments(UINT32_MAX, true, UINT32_MAX) == 71);
    assert(refiz_rpm_lit_segments(UINT32_MAX, false, UINT32_MAX) == 65);
    assert(refiz_rpm_lit_segments(8000, true, 6000) == 61);
    assert(refiz_rpm_lit_segments(8000, false, 6000) == 49);
    assert(refiz_rpm_lit_segments(8000, false, 0) == 1);
    puts("PASS: saturation and existing maximum-RPM input clamp");
}

static void check_payload(const uint8_t *payload, unsigned lit, uint8_t untouched)
{
    for (unsigned bit = 0; bit < 520; ++bit) {
        bool expected = !!(untouched & (1u << (bit & 7)));
        for (unsigned i = 0; i < 71; ++i) {
            if (refiz_rpm_segments[i] + 3u == bit) expected = i < lit;
        }
        assert(!!(payload[bit >> 3] & (1u << (bit & 7))) == expected);
    }
}

static void test_payload_switching(void)
{
    uint8_t payload[65];
    memset(payload, 0xA5, sizeof(payload));
    assert(refiz_rpm_segments[0] == 64);
    assert(refiz_rpm_segments[70] == 96);
    refiz_rpm_write_payload(payload, refiz_rpm_lit_segments(8000, true, 8000));
    check_payload(payload, 71, 0xA5);
    refiz_rpm_write_payload(payload, refiz_rpm_lit_segments(8000, false, 8000));
    check_payload(payload, 65, 0xA5); // All six unused upper marks are cleared.
    refiz_rpm_write_payload(payload, refiz_rpm_lit_segments(1000, false, 8000));
    check_payload(payload, 9, 0xA5);
    refiz_rpm_write_payload(payload, refiz_rpm_lit_segments(7000, true, 8000));
    check_payload(payload, 71, 0xA5);
    refiz_rpm_write_payload(payload, refiz_rpm_lit_segments(0, true, 8000));
    check_payload(payload, 1, 0xA5);
    puts("PASS: live mode changes/falling RPM clear the bar without changing other payload bits");
}
#endif

int main(void)
{
#ifdef TEST_NEXT_DEFAULTS
    assert(defaults.rpmOptions_7k_line.value == 0);
    puts("PASS: Digifiz Next 7k parameter default remains unchanged");
#else
    assert(defaults.rpmOptions_7k_line.value == 1);
    puts("PASS: actual ReFIZ parameter table defaults to 7k");
    test_step_boundaries();
    test_saturation_and_maximum();
    test_payload_switching();
#endif
    return 0;
}
