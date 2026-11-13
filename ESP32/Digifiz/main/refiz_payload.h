#ifndef REFIZ_PAYLOAD_H
#define REFIZ_PAYLOAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define REFIZ_PAYLOAD_BITS 520u
#define REFIZ_PAYLOAD_BYTES (REFIZ_PAYLOAD_BITS / 8u)
#define REFIZ_NORMAL_BITS 264u
#define REFIZ_LOGICAL_SEGMENTS 261u
#define REFIZ_MAX_FRAME_BYTES (10u + REFIZ_PAYLOAD_BYTES)

/* Original ESP32 default pattern, also returned to the editor. Unspecified
 * bytes are zero. There is no separate default pattern in JavaScript. */
static const uint8_t refiz_default_payload[REFIZ_PAYLOAD_BYTES] = {
    [0] = 0x07, [37] = 0xF8, [38] = 0x3B, [39] = 0xFE, [40] = 0x06,
    [44] = 0xF8, [45] = 0xFB, [46] = 0xEF, [47] = 0xFD, [48] = 0x06,
    [64] = 0xF8
};

typedef struct {
    bool initialized;
    bool enabled;
    bool full_refresh;
    uint8_t payload[REFIZ_PAYLOAD_BYTES];
} refiz_diagnostic_state_t;

static inline void refiz_diagnostic_ensure_initialized(refiz_diagnostic_state_t *state)
{
    if (!state->initialized) {
        memcpy(state->payload, refiz_default_payload, REFIZ_PAYLOAD_BYTES);
        state->initialized = true;
        state->full_refresh = true;
    }
}

/* Caller holds the diagnostic lock. Entering diagnostics starts from defaults;
 * an explicit payload can then replace all bits in the same atomic update. */
static inline void refiz_diagnostic_update(refiz_diagnostic_state_t *state,
                                           bool enabled, const uint8_t *payload, bool reset)
{
    refiz_diagnostic_ensure_initialized(state);
    if (reset || (enabled && !state->enabled))
        memcpy(state->payload, refiz_default_payload, REFIZ_PAYLOAD_BYTES);
    if (payload) memcpy(state->payload, payload, REFIZ_PAYLOAD_BYTES);
    if (enabled != state->enabled) state->full_refresh = true;
    state->enabled = enabled;
}

/* Snapshot while locked, then render/transmit outside the lock. Send a full
 * normal frame on exit to restore any changed upper configuration bytes. */
static inline bool refiz_diagnostic_prepare(refiz_diagnostic_state_t *state,
                                            uint8_t *payload, bool *enabled)
{
    refiz_diagnostic_ensure_initialized(state);
    *enabled = state->enabled;
    memcpy(payload, state->enabled ? state->payload : refiz_default_payload, REFIZ_PAYLOAD_BYTES);
    bool full = state->enabled || state->full_refresh;
    state->full_refresh = false;
    return full;
}

/* Existing SLCD 0x03 SET_PACKED_RANGE protocol: offset zero, LSB-first bytes. */
static inline size_t refiz_build_payload_frame(uint8_t *frame, const uint8_t *payload, bool full)
{
    uint16_t bits = full ? REFIZ_PAYLOAD_BITS : REFIZ_NORMAL_BITS;
    size_t bytes = bits / 8u;
    uint16_t length = 4u + bytes;
    frame[0] = 0xA5;
    frame[1] = 0x5A;
    frame[2] = 0x03;
    frame[3] = length & 0xFF;
    frame[4] = length >> 8;
    frame[5] = frame[6] = 0;
    frame[7] = bits & 0xFF;
    frame[8] = bits >> 8;
    memcpy(frame + 9, payload, bytes);
    uint8_t crc = 0;
    for (size_t i = 2; i < 9 + bytes; ++i) crc ^= frame[i];
    frame[9 + bytes] = crc;
    return 10 + bytes;
}

#endif
