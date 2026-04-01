#include "pattern_sender.h"

volatile uint8_t tr_status = 0x00; // sending, clockbit, 000000
volatile uint16_t data_cnt = 0;
volatile uint8_t bool_data_payload[PAYLOAD_SIZE] = {
  1,1,1,
  0, 1, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 1, 1, 0, 0, 1, 0, 1,
  0, 1, 1, 1, 1, 0, 1, 1,
  1, 1, 1, 0, 1, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 1, 1, 1,
  1, 1, 0, 1, 0, 1, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0,
  1, 1, 0, 0, 0, 1, 0, 1,
  0, 1, 1, 1, 0, 1, 1, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 1, 1, 1, 1,
  1, 0, 0, 0, 1, 0, 1, 1,
  1, 0, 1, 1, 0, 1, 0, 0,
  0, 1, 1, 1, 0, 1, 1, 1,
  1, 0, 1, 1, 1, 0, 1, 1,
  1, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 0,
  1, 1, 1, 0, 0, 0, 1, 1,
  1, 1, 1, 1, 1, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 1, 1, 1, 1, 0, 1,
  1, 1, 1, 1, 1, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1,1,1,1,1
};

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
        if (bool_data_payload[data_cnt]) {
          DATA_PORT |= (1 << DATA_PIN_NAME);
        } else {
          DATA_PORT &= ~(1 << DATA_PIN_NAME);
        }
      } else if (data_cnt > 3) {
        delayMicroseconds(8);
        if (bool_data_payload[data_cnt]) {
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
  for (uint16_t i = 0; i < PAYLOAD_SIZE; ++i) {
    bool_data_payload[i] = (i < len && src[i] != 0) ? 1 : 0;
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
    bool_data_payload[offset + i] = (src[i] != 0) ? 1 : 0;
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
    dst[i] = bool_data_payload[offset + i];
  }
  interrupts();
}
