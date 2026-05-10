#include "pattern_sender.h"

#include <string.h>

volatile uint8_t tr_status = 0x00; // sending, clockbit, 000000
volatile uint16_t data_cnt = 0;
volatile uint8_t bool_data_payload[PACKED_PAYLOAD_SIZE] = {
  0x97, 0xFF, 0xF3, 0xF9, 0x37, 0xF5, 0xBE, 0xF8, 0xFF, 0x1F, 0x78, 0x5F, 0xF9,
  0x1F, 0x18, 0x75, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xF7, 0x8F,
  0x8F, 0x6E, 0x71, 0xEF, 0x8E, 0xFF, 0x05, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x3B,
  0xFE, 0x06, 0x00, 0x00, 0x00, 0xF8, 0xFB, 0xEF, 0xFD, 0x06, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8
};

static bool getPayloadBit(uint16_t bit_index) {
  if (bit_index >= PAYLOAD_SIZE) {
    return false;
  }
  return (bool_data_payload[bit_index >> 3] & (1 << (bit_index & 0x07))) != 0;
}

static void setPayloadBit(uint16_t bit_index, bool value) {
  if (bit_index >= PAYLOAD_SIZE) {
    return;
  }
  const uint8_t mask = 1 << (bit_index & 0x07);
  if (value) {
    bool_data_payload[bit_index >> 3] |= mask;
  } else {
    bool_data_payload[bit_index >> 3] &= ~mask;
  }
}

void initDisplayClockTimer() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 243;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  if ((tr_status & 0x40) == 0) {
    if (CLOCK_PORT & (1 << CLOCK_PIN_NAME)) {
      CLOCK_PORT &= ~(1 << CLOCK_PIN_NAME);
      if ((tr_status & 0x10) == 0) {
        STROBE_CLOCK_PORT |= (1 << STROBE_CLOCK_PIN_NAME);
      } else {
        DATA_PORT |= (1 << DATA_PIN_NAME);
        delayMicroseconds(16);
        DATA_PORT &= ~(1 << DATA_PIN_NAME);
      }
    } else {
      if (((tr_status & 0x20) == 0) && ((tr_status & 0x10) == 0)) {
        STROBE_CLOCK_PORT |= (1 << STROBE_CLOCK_PIN_NAME);
      }
      CLOCK_PORT |= (1 << CLOCK_PIN_NAME);
      if (tr_status & 0x20) {
        tr_status &= ~0x20;
        tr_status |= 0x10;
        delayMicroseconds(8);
        STROBE_CLOCK_PORT &= ~(1 << STROBE_CLOCK_PIN_NAME);
        delayMicroseconds(2);
        DATA_PORT &= ~(1 << DATA_PIN_NAME);
      } else if (tr_status & 0x10) {
        tr_status &= ~0x10;
        DATA_PORT |= (1 << DATA_PIN_NAME);
        delayMicroseconds(8);
        STROBE_CLOCK_PORT |= (1 << STROBE_CLOCK_PIN_NAME);
      }
    }

    if (tr_status & 0x80) {
      delayMicroseconds(8);
      if (data_cnt == 3) {
        DATA_PORT &= ~(1 << DATA_PIN_NAME);
      }
      if (data_cnt == 515) {
        DATA_PORT |= (1 << DATA_PIN_NAME);
      }
    }
  } else {
    if (tr_status & 0x80) {
      STROBE_CLOCK_PORT &= ~(1 << STROBE_CLOCK_PIN_NAME);

      if (data_cnt > 250) {
        delayMicroseconds(8);
        if (getPayloadBit(data_cnt)) {
          DATA_PORT |= (1 << DATA_PIN_NAME);
        } else {
          DATA_PORT &= ~(1 << DATA_PIN_NAME);
        }
      } else if (data_cnt > 3) {
        delayMicroseconds(8);
        if (getPayloadBit(data_cnt)) {
          DATA_PORT |= (1 << DATA_PIN_NAME);
        } else {
          DATA_PORT &= ~(1 << DATA_PIN_NAME);
        }
      }

      data_cnt++;
      if (data_cnt == 516) {
        data_cnt = 0;
        tr_status &= ~0x80;
        DATA_PORT |= (1 << DATA_PIN_NAME);
      }
    }
  }
  tr_status ^= 0x40;
}

void initDigifiz() {
  tr_status |= 0x20;
}

void initPins() {
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(STROBE_CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(CLOCK_PIN, HIGH);
  digitalWrite(STROBE_CLOCK_PIN, HIGH);
  digitalWrite(DATA_PIN, HIGH);
}

void triggerFrameIfIdle() {
  if (!(tr_status & 0x80)) {
    tr_status |= 0x80;
  }
}

void setPayloadAll(const uint8_t* src, uint16_t len) {
  if (len > PAYLOAD_SIZE) {
    len = PAYLOAD_SIZE;
  }
  noInterrupts();
  memset((void*)bool_data_payload, 0, PACKED_PAYLOAD_SIZE);
  for (uint16_t i = 0; i < len; ++i) {
    setPayloadBit(i, src[i] != 0);
  }
  interrupts();
}

void setPayloadRange(uint16_t offset, const uint8_t* src, uint16_t len) {
  if (offset >= PAYLOAD_SIZE || len == 0) {
    return;
  }
  if (offset + len > PAYLOAD_SIZE) {
    len = PAYLOAD_SIZE - offset;
  }
  noInterrupts();
  for (uint16_t i = 0; i < len; ++i) {
    setPayloadBit(offset + i, src[i] != 0);
  }
  interrupts();
}

void setPayloadPackedRange(uint16_t offset, const uint8_t* src, uint16_t bit_len) {
  if (offset >= PAYLOAD_SIZE || bit_len == 0 || src == NULL) {
    return;
  }

  if (offset + bit_len > PAYLOAD_SIZE) {
    bit_len = PAYLOAD_SIZE - offset;
  }

  noInterrupts();

  // Fast path: byte-aligned copy
  if ((offset & 0x07) == 0) {
    uint16_t byte_offset = offset >> 3;
    uint16_t full_bytes = bit_len >> 3;
    uint8_t remaining_bits = bit_len & 0x07;

    if (full_bytes > 0) {
      memcpy((void*)&bool_data_payload[byte_offset], src, full_bytes);
    }

    // Copy remaining bits without touching upper bits
    if (remaining_bits > 0) {
      uint8_t mask = (1 << remaining_bits) - 1;
      bool_data_payload[byte_offset + full_bytes] =
        (bool_data_payload[byte_offset + full_bytes] & ~mask) |
        (src[full_bytes] & mask);
    }

    interrupts();
    return;
  }

  // Fallback: unaligned bit copy
  for (uint16_t i = 0; i < bit_len; ++i) {
    const bool value = (src[i >> 3] & (1 << (i & 0x07))) != 0;
    setPayloadBit(offset + i, value);
  }

  interrupts();
}

uint16_t getPayloadSize() {
  return PAYLOAD_SIZE;
}

void getPayloadRange(uint16_t offset, uint8_t* dst, uint16_t len) {
  if (offset >= PAYLOAD_SIZE || len == 0) {
    return;
  }
  if (offset + len > PAYLOAD_SIZE) {
    len = PAYLOAD_SIZE - offset;
  }
  noInterrupts();
  for (uint16_t i = 0; i < len; ++i) {
    dst[i] = getPayloadBit(offset + i) ? 1 : 0;
  }
  interrupts();
}
