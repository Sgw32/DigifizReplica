#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OSC_CAPTURE_SECONDS_NUM 1U
#define OSC_CAPTURE_SECONDS_DEN 4U
#define OSC_SAMPLE_RATE_HZ 20000U
#define OSC_SAMPLE_COUNT ((OSC_SAMPLE_RATE_HZ * OSC_CAPTURE_SECONDS_NUM) / OSC_CAPTURE_SECONDS_DEN)
#define OSC_PACKED_WORDS ((OSC_SAMPLE_COUNT + 31U) / 32U)

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t sample_rate_hz;
    uint32_t sample_count;
    uint32_t packed_words;
    uint32_t write_index;
    uint32_t wrapped;
} oscilloscope_dump_header_t;

void oscilloscope_init(void);
void oscilloscope_deinit(void);
esp_err_t oscilloscope_build_dump(uint8_t **out_buf, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif
