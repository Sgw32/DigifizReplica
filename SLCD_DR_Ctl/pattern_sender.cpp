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

namespace {

bool g_interactive_mode = false;
bool g_interactive_float_dot = false;
uint16_t g_interactive_max_rpm = 7000;

const uint8_t kDigitMask[10] = {
  0b00111111, 0b00100001, 0b01011011, 0b01110011, 0b01100101,
  0b01110110, 0b01111110, 0b00100011, 0b01111111, 0b01110111
};

const uint8_t kRpmSegments[71] = {
  64,65,66,67,68,69,70,71,72,73,24,25,26,27,28,29,30,31,
  18,17,159,158,157,156,155,154,153,152,151,150,149,148,147,
  146,145,253,177,178,179,180,181,182,183,184,185,186,187,188,
  189,190,191,49,50,63,62,61,60,59,58,57,56,105,104,103,102,
  101,100,99,98,97,96
};

void writeBitValue(uint16_t orig_index, bool value) {
  setPayloadBit(orig_index + 3, value);
}

void writeDigit(const uint16_t segments[7], uint8_t digit, bool blank) {
  uint8_t mask = blank ? 0 : kDigitMask[digit % 10];
  for (uint8_t i = 0; i < 7; ++i) {
    writeBitValue(segments[i], (mask & (1 << i)) != 0);
  }
}

void setInteractiveMfaType(uint8_t type) {
  writeBitValue(51, type == 0);  // duration
  writeBitValue(19, type == 1);  // km
  writeBitValue(52, type == 2);  // l/100
  writeBitValue(20, type == 3);  // km/h
  writeBitValue(21, type == 4);  // oil
  writeBitValue(53, type == 5);  // air
}

void setInteractiveMileage(uint32_t mileage) {
  const uint16_t s1[7] = {75,74,74,74,74,107,106};
  const uint16_t s2[7] = {79,76,77,109,110,111,78};
  const uint16_t s3[7] = {83,80,81,113,114,115,82};
  const uint16_t s4[7] = {87,84,85,117,118,119,86};
  const uint16_t s5[7] = {91,88,89,121,122,123,90};
  const uint16_t s6[7] = {95,92,93,125,126,127,94};
  const uint16_t* segs[6] = {s1, s2, s3, s4, s5, s6};
  uint8_t digits[6];
  for (int8_t i = 5; i >= 0; --i) { digits[i] = mileage % 10; mileage /= 10; }
  bool nonzero = false;
  for (uint8_t i = 0; i < 6; ++i) {
    nonzero = nonzero || digits[i] != 0 || i == 5;
    writeDigit(segs[i], digits[i], !nonzero);
  }
}

void setInteractiveClock(uint8_t hours, uint8_t minutes) {
  const uint16_t s1[7] = {3,0,1,33,34,35,2};
  const uint16_t s2[7] = {7,4,5,37,38,39,6};
  const uint16_t s3[7] = {11,8,9,41,42,43,10};
  const uint16_t s4[7] = {15,12,13,45,46,47,14};
  writeDigit(s1, (hours / 10) % 10, hours < 10);
  writeDigit(s2, hours % 10, false);
  writeDigit(s3, (minutes / 10) % 10, false);
  writeDigit(s4, minutes % 10, false);
}

void setInteractiveMfaClock(uint8_t hours, uint8_t minutes) {
  const uint16_t s1[7] = {195,192,193,225,226,227,194};
  const uint16_t s2[7] = {199,196,197,229,230,231,198};
  const uint16_t s3[7] = {204,201,202,234,235,236,203};
  const uint16_t s4[7] = {208,205,206,238,239,240,207};
  if (hours > 99) { hours = 99; minutes = 99; }
  writeDigit(s1, (hours / 10) % 10, hours < 10);
  writeDigit(s2, hours % 10, false);
  writeDigit(s3, (minutes / 10) % 10, false);
  writeDigit(s4, minutes % 10, false);
}

void setInteractiveMfaNumber(int16_t data) {
  const uint16_t s1[7] = {195,192,193,225,226,227,194};
  const uint16_t s2[7] = {199,196,197,229,230,231,198};
  const uint16_t s3[7] = {204,201,202,234,235,236,203};
  const uint16_t s4[7] = {208,205,206,238,239,240,207};
  writeBitValue(229, g_interactive_float_dot);
  bool neg = data < 0;
  uint16_t v = neg ? -data : data;
  if (neg) {
    writeDigit(s1, 0, v < 100);
    if (v >= 100) writeDigit(s1, (v / 1000) % 10, v < 1000);
    uint8_t minus = 0b01000000;
    for (uint8_t i = 0; i < 7; ++i) writeBitValue(s2[i], v < 1000 && v >= 100 ? (kDigitMask[(v / 100) % 10] & (1 << i)) : (minus & (1 << i)));
  } else {
    writeDigit(s1, (v / 1000) % 10, v < 1000);
    writeDigit(s2, (v / 100) % 10, false);
  }
  writeDigit(s3, (v / 10) % 10, false);
  writeDigit(s4, v % 10, false);
}

void setInteractiveFuel(uint8_t litres) {
  const uint16_t s1[7] = {129,222,223,255,160,161,128};
  const uint16_t s2[7] = {133,130,131,163,164,165,132};
  writeDigit(s1, (litres / 10) % 10, litres < 10);
  writeDigit(s2, litres % 10, false);
}

void setInteractiveSpeed(uint16_t speed) {
  const uint16_t s1[7] = {252,221,221,221,221,220,221};
  const uint16_t s2[7] = {250,249,248,218,219,216,217};
  const uint16_t s3[7] = {244,245,246,214,215,212,213};
  writeDigit(s1, (speed / 100) % 10, speed < 100);
  writeDigit(s2, (speed / 10) % 10, speed < 10);
  writeDigit(s3, speed % 10, false);
}

void setInteractiveRpm(uint16_t rpm_value) {
  uint32_t rpm = rpm_value;
  rpm *= 70;
  rpm /= g_interactive_max_rpm == 0 ? 7000 : g_interactive_max_rpm;
  if (rpm > 70) rpm = 70;
  for (uint8_t i = 0; i < 70; ++i) writeBitValue(kRpmSegments[i], i <= rpm);
}

void setInteractiveCoolant(uint16_t data) {
  const uint16_t segs[20] = {143,142,141,140,139,138,137,136,135,134,166,167,168,169,170,171,172,173,174,175};
  if (data > 20) data = 20;
  for (uint8_t i = 0; i < 20; ++i) writeBitValue(segs[i], i < data);
}

}  // namespace

void setInteractiveMode(bool enabled) {
  g_interactive_mode = enabled;
}

bool isInteractiveMode() {
  return g_interactive_mode;
}

bool applyInteractiveField(uint8_t field, int32_t value) {
  if (!g_interactive_mode && field != IF_MAX_RPM) {
    return false;
  }

  noInterrupts();
  switch (field) {
    case IF_SPEED: setInteractiveSpeed((uint16_t)value); break;
    case IF_RPM: setInteractiveRpm((uint16_t)value); break;
    case IF_MFA_TYPE: setInteractiveMfaType((uint8_t)value); break;
    case IF_MFA_NUMBER: setInteractiveMfaNumber((int16_t)value); break;
    case IF_MFA_CLOCK: setInteractiveMfaClock((uint8_t)(value >> 8), (uint8_t)value); break;
    case IF_FUEL: setInteractiveFuel((uint8_t)value); break;
    case IF_COOLANT: setInteractiveCoolant((uint16_t)value); break;
    case IF_MILEAGE: setInteractiveMileage((uint32_t)value); break;
    case IF_CLOCK: setInteractiveClock((uint8_t)(value >> 8), (uint8_t)value); break;
    case IF_DOT: writeBitValue(40, value != 0); break;
    case IF_FLOAT_DOT: g_interactive_float_dot = value != 0; break;
    case IF_REFUEL_SIGN: writeBitValue(176, value != 0); break;
    case IF_MFA_BLOCK: break;  // External MFA block pins are not present on SLCD_DR_Ctl.
    case IF_BACKLIGHT: break;  // External backlight pin is not present on SLCD_DR_Ctl.
    case IF_MAX_RPM: g_interactive_max_rpm = value > 0 ? (uint16_t)value : 7000; break;
    default:
      interrupts();
      return false;
  }
  interrupts();
  return true;
}
