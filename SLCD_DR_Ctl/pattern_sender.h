#pragma once

#include <Arduino.h>
#include "config.h"

extern volatile uint8_t tr_status;
extern volatile uint16_t data_cnt;
extern volatile uint8_t bool_data_payload[PAYLOAD_SIZE];

void initDisplayClockTimer();
void initDigifiz();
void initPins();
void triggerFrameIfIdle();

void setPayloadAll(const uint8_t* src, uint16_t len);
void setPayloadRange(uint16_t offset, const uint8_t* src, uint16_t len);
uint16_t getPayloadSize();
void getPayloadRange(uint16_t offset, uint8_t* dst, uint16_t len);
