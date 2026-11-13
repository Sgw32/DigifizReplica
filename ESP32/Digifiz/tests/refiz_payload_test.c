#include "refiz_payload.h"
#include <assert.h>
#include <stdio.h>

static void check_frame(const uint8_t *frame, size_t size, const uint8_t *payload, unsigned bits)
{
    assert(frame[0] == 0xA5 && frame[1] == 0x5A && frame[2] == 0x03);
    assert((unsigned)(frame[3] | frame[4] << 8) == 4 + bits / 8);
    assert(frame[5] == 0 && frame[6] == 0);
    assert((unsigned)(frame[7] | frame[8] << 8) == bits);
    assert(size == 10 + bits / 8);
    assert(memcmp(frame + 9, payload, bits / 8) == 0);
    uint8_t xor = 0;
    for (size_t i = 2; i < size; ++i) xor ^= frame[i];
    assert(xor == 0);
}

static void test_defaults_and_activation(void)
{
    refiz_diagnostic_state_t state = {0};
    refiz_diagnostic_ensure_initialized(&state);
    assert(!state.enabled);
    assert(state.payload[0] == 7);
    assert(state.payload[37] == 0xF8 && state.payload[38] == 0x3B);
    assert(state.payload[64] == 0xF8);
    refiz_diagnostic_update(&state, true, NULL, false);
    assert(state.enabled && memcmp(state.payload, refiz_default_payload, 65) == 0);
    memset(state.payload, 0x55, 65);
    refiz_diagnostic_update(&state, false, NULL, false);
    refiz_diagnostic_update(&state, true, NULL, false);
    assert(memcmp(state.payload, refiz_default_payload, 65) == 0);
    memset(state.payload, 0, 65);
    refiz_diagnostic_update(&state, true, NULL, true);
    assert(memcmp(state.payload, refiz_default_payload, 65) == 0);
    puts("PASS: activation and reset use the original full default pattern");
}

static void test_every_bit(void)
{
    refiz_diagnostic_state_t state = {0};
    for (unsigned bit = 0; bit < 520; ++bit) {
        uint8_t selected[65] = {0}, snapshot[65], frame[REFIZ_MAX_FRAME_BYTES];
        selected[bit / 8] = 1u << (bit % 8);
        refiz_diagnostic_update(&state, true, selected, false);
        bool enabled;
        assert(refiz_diagnostic_prepare(&state, snapshot, &enabled));
        assert(enabled);
        size_t size = refiz_build_payload_frame(frame, snapshot, true);
        check_frame(frame, size, selected, 520);
        // Interpret it as the SLCD SET_PACKED_RANGE receiver does, LSB first.
        for (unsigned received = 0; received < 520; ++received)
            assert(!!(frame[9 + received / 8] & (1u << (received % 8))) == (received == bit));
    }
    puts("PASS: all 520 raw bits reach the UART frame, including bits 0..2 and 519");
}

static void test_full_masks_and_snapshot(void)
{
    refiz_diagnostic_state_t state = {0};
    uint8_t selected[65], snapshot[65], frame[REFIZ_MAX_FRAME_BYTES];
    for (unsigned value = 0; value <= 255; value += 255) {
        memset(selected, value, 65);
        refiz_diagnostic_update(&state, true, selected, false);
        bool enabled;
        assert(refiz_diagnostic_prepare(&state, snapshot, &enabled) && enabled);
        check_frame(frame, refiz_build_payload_frame(frame, snapshot, true), selected, 520);
        // A later HTTP edit cannot mutate the already snapshotted UART data.
        uint8_t later[65];
        memset(later, value ^ 255, 65);
        refiz_diagnostic_update(&state, true, later, false);
        assert(memcmp(snapshot, selected, 65) == 0);
    }
    puts("PASS: clear/select all and snapshots preserve entire independent frames");
}

static void test_exit_restores_upper_bytes(void)
{
    refiz_diagnostic_state_t state = {0};
    uint8_t selected[65] = {0}, snapshot[65], frame[REFIZ_MAX_FRAME_BYTES];
    bool enabled;
    // Startup also synchronizes controller configuration with a full frame.
    assert(refiz_diagnostic_prepare(&state, snapshot, &enabled) && !enabled);
    assert(!refiz_diagnostic_prepare(&state, snapshot, &enabled));
    refiz_diagnostic_update(&state, true, selected, false);
    assert(refiz_diagnostic_prepare(&state, snapshot, &enabled) && enabled);
    assert(refiz_diagnostic_prepare(&state, snapshot, &enabled));
    refiz_diagnostic_update(&state, false, NULL, false);
    assert(refiz_diagnostic_prepare(&state, snapshot, &enabled) && !enabled);
    assert(memcmp(snapshot, refiz_default_payload, 65) == 0);
    check_frame(frame, refiz_build_payload_frame(frame, snapshot, true), refiz_default_payload, 520);
    assert(!refiz_diagnostic_prepare(&state, snapshot, &enabled));
    check_frame(frame, refiz_build_payload_frame(frame, snapshot, false), refiz_default_payload, 264);
    puts("PASS: disabling restores all upper bytes once, then normal 264-bit updates resume");
}

int main(void)
{
    test_defaults_and_activation();
    test_every_bit();
    test_full_masks_and_snapshot();
    test_exit_restores_upper_bytes();
    return 0;
}
