#pragma once

#include <Arduino.h>

// Pin mapping (Arduino Nano / ATmega328P)
constexpr uint8_t CLOCK_PIN = 8;
constexpr uint8_t STROBE_CLOCK_PIN = 9;
constexpr uint8_t DATA_PIN = 10;

constexpr uint8_t CLOCK_PIN_NAME = PB0;
constexpr uint8_t STROBE_CLOCK_PIN_NAME = PB1;
constexpr uint8_t DATA_PIN_NAME = PB2;

#define CLOCK_PORT PORTB
#define STROBE_CLOCK_PORT PORTB
#define DATA_PORT PORTB

constexpr uint16_t PAYLOAD_SIZE = 520;
constexpr uint16_t PACKED_PAYLOAD_SIZE = (PAYLOAD_SIZE + 7) / 8;
constexpr uint32_t UART_BAUD = 57600;
