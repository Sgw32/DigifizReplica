#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t tablCrc8[256];
extern const uint8_t tablCrc8_dvbs2[256];

uint8_t crc8(const uint8_t *crcbuff, uint16_t crcLen);
uint8_t crc8_dvbs2(const uint8_t *crcbuff, uint16_t crcLen);

#ifdef __cplusplus
}
#endif
