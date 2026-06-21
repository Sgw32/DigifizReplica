#pragma once

#include <Arduino.h>
#include "config.h"

extern volatile uint8_t tr_status;
extern volatile uint16_t data_cnt;
extern volatile uint8_t bool_data_payload[PACKED_PAYLOAD_SIZE];

void initDisplayClockTimer();
void initDigifiz();
void initPins();
void triggerFrameIfIdle();

void setPayloadAll(const uint8_t* src, uint16_t len);
void setPayloadRange(uint16_t offset, const uint8_t* src, uint16_t len);
void setPayloadPackedRange(uint16_t offset, const uint8_t* src, uint16_t bit_len);
uint16_t getPayloadSize();
void getPayloadRange(uint16_t offset, uint8_t* dst, uint16_t len);

enum InteractiveField : uint8_t {
  IF_SPEED = 0x01,
  IF_RPM = 0x02,
  IF_MFA_TYPE = 0x03,
  IF_MFA_NUMBER = 0x04,
  IF_MFA_CLOCK = 0x05,
  IF_FUEL = 0x06,
  IF_COOLANT = 0x07,
  IF_MILEAGE = 0x08,
  IF_CLOCK = 0x09,
  IF_DOT = 0x0A,
  IF_FLOAT_DOT = 0x0B,
  IF_REFUEL_SIGN = 0x0C,
  IF_MFA_BLOCK = 0x0D,
  IF_BACKLIGHT = 0x0E,
  IF_MAX_RPM = 0x0F
};

void setInteractiveMode(bool enabled);
bool isInteractiveMode();
bool applyInteractiveField(uint8_t field, int32_t value);
