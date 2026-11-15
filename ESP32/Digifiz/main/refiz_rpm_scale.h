#ifndef REFIZ_RPM_SCALE_H
#define REFIZ_RPM_SCALE_H

#include <stdbool.h>
#include <stdint.h>

/* A fixed zero marker followed by 70 steps (7k) or 64 steps (8k). */
#define REFIZ_RPM_STEPS_7K 70u
#define REFIZ_RPM_STEPS_8K 64u

static const uint16_t refiz_rpm_segments[REFIZ_RPM_STEPS_7K + 1] = {
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    24, 25, 26, 27, 28, 29, 30, 31, 18, 17,
    159, 158, 157, 156, 155, 154, 153, 152, 151, 150,
    149, 148, 147, 146, 145, 253, 177, 178, 179, 180,
    181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
    191, 49, 50, 63, 62, 61, 60, 59, 58, 57,
    56, 105, 104, 103, 102, 101, 100, 99, 98, 97,
    96
};

static inline uint8_t refiz_rpm_lit_segments(uint32_t rpm, bool seven_k, uint32_t maximum_rpm)
{
    if (rpm > maximum_rpm) rpm = maximum_rpm;
    uint32_t steps = rpm / (seven_k ? 100u : 125u);
    uint32_t maximum_steps = seven_k ? REFIZ_RPM_STEPS_7K : REFIZ_RPM_STEPS_8K;
    if (steps > maximum_steps) steps = maximum_steps;
    return (uint8_t)(steps + 1u); // Zero marker stays on, including at zero RPM.
}

/* Rewrite the complete physical bar, clearing segments left over by a change
 * from 7k to 8k or by a falling RPM. Leave unrelated payload bits intact. */
static inline void refiz_rpm_write_payload(uint8_t *payload, uint8_t lit_segments)
{
    for (unsigned i = 0; i < REFIZ_RPM_STEPS_7K + 1; ++i) {
        unsigned bit = refiz_rpm_segments[i] + 3u;
        uint8_t mask = 1u << (bit & 7u);
        if (i < lit_segments) payload[bit >> 3] |= mask;
        else payload[bit >> 3] &= (uint8_t)~mask;
    }
}

#endif
