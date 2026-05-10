#include "serial_protocol.h"

#include "pattern_sender.h"

namespace {
constexpr uint8_t SOF1 = 0xA5;
constexpr uint8_t SOF2 = 0x5A;
constexpr uint8_t CMD_SET_ALL = 0x01;
constexpr uint8_t CMD_SET_RANGE = 0x02;
constexpr uint8_t CMD_SET_PACKED_RANGE = 0x03;
constexpr uint8_t CMD_GET_INFO = 0x10;
constexpr uint8_t CMD_GET_RANGE = 0x11;

constexpr uint8_t RSP_ACK = 0x80;
constexpr uint8_t RSP_NACK = 0x81;
constexpr uint8_t RSP_INFO = 0x90;
constexpr uint8_t RSP_RANGE = 0x91;

constexpr uint16_t MAX_FRAME_PAYLOAD = 600;

enum class ParseState : uint8_t {
  WaitSof1,
  WaitSof2,
  WaitCmd,
  WaitLenL,
  WaitLenH,
  WaitPayload,
  WaitCrc
};

Stream* g_serial = nullptr;
ParseState g_state = ParseState::WaitSof1;
uint8_t g_cmd = 0;
uint16_t g_len = 0;
uint16_t g_pos = 0;
uint8_t g_crc = 0;
uint8_t g_payload[MAX_FRAME_PAYLOAD];

uint8_t crc8(const uint8_t* data, uint16_t len) {
  uint8_t c = 0;
  for (uint16_t i = 0; i < len; ++i) {
    c ^= data[i];
  }
  return c;
}

void resetParser() {
  g_state = ParseState::WaitSof1;
  g_cmd = 0;
  g_len = 0;
  g_pos = 0;
  g_crc = 0;
}

void sendFrame(uint8_t cmd, const uint8_t* payload, uint16_t len) {
  if (g_serial == nullptr) {
    return;
  }
  uint8_t header[3] = {cmd, static_cast<uint8_t>(len & 0xFF), static_cast<uint8_t>((len >> 8) & 0xFF)};
  uint8_t c = crc8(header, 3) ^ crc8(payload, len);

  g_serial->write(SOF1);
  g_serial->write(SOF2);
  g_serial->write(cmd);
  g_serial->write(static_cast<uint8_t>(len & 0xFF));
  g_serial->write(static_cast<uint8_t>((len >> 8) & 0xFF));
  if (len > 0) {
    g_serial->write(payload, len);
  }
  g_serial->write(c);
}

void sendAck(uint8_t commandEcho) {
  //uint8_t payload[1] = {commandEcho};
  //sendFrame(RSP_ACK, payload, 1);
}

void sendNack(uint8_t errCode) {
  uint8_t payload[1] = {errCode};
  sendFrame(RSP_NACK, payload, 1);
}

void handleFrame(uint8_t cmd, const uint8_t* payload, uint16_t len) {
  switch (cmd) {
    case CMD_SET_ALL:
      setPayloadAll(payload, len);
      sendAck(CMD_SET_ALL);
      break;

    case CMD_SET_RANGE: {
      if (len < 4) {
        sendNack(0x01);
        break;
      }
      const uint16_t offset = static_cast<uint16_t>(payload[0]) | (static_cast<uint16_t>(payload[1]) << 8);
      const uint16_t count = static_cast<uint16_t>(payload[2]) | (static_cast<uint16_t>(payload[3]) << 8);
      if (len != static_cast<uint16_t>(count + 4)) {
        sendNack(0x02);
        break;
      }
      setPayloadRange(offset, &payload[4], count);
      sendAck(CMD_SET_RANGE);
      break;
    }

    case CMD_SET_PACKED_RANGE: {
      if (len < 4) {
        sendNack(0x07);
        break;
      }
      const uint16_t offset = static_cast<uint16_t>(payload[0]) | (static_cast<uint16_t>(payload[1]) << 8);
      const uint16_t bit_count = static_cast<uint16_t>(payload[2]) | (static_cast<uint16_t>(payload[3]) << 8);
      const uint16_t byte_count = (bit_count + 7) / 8;
      if (len != static_cast<uint16_t>(byte_count + 4)) {
        sendNack(0x08);
        break;
      }
      setPayloadPackedRange(offset, &payload[4], bit_count);
      sendAck(CMD_SET_PACKED_RANGE);
      break;
    }

    case CMD_GET_INFO: {
      uint8_t info[2] = {
          static_cast<uint8_t>(getPayloadSize() & 0xFF),
          static_cast<uint8_t>((getPayloadSize() >> 8) & 0xFF)};
      sendFrame(RSP_INFO, info, 2);
      break;
    }

    case CMD_GET_RANGE: {
      if (len != 4) {
        sendNack(0x03);
        break;
      }
      const uint16_t offset = static_cast<uint16_t>(payload[0]) | (static_cast<uint16_t>(payload[1]) << 8);
      const uint16_t count = static_cast<uint16_t>(payload[2]) | (static_cast<uint16_t>(payload[3]) << 8);
      if (count > PAYLOAD_SIZE) {
        sendNack(0x04);
        break;
      }
      uint8_t out[MAX_FRAME_PAYLOAD];
      uint16_t n = count;
      if (offset + n > PAYLOAD_SIZE) {
        n = PAYLOAD_SIZE - offset;
      }
      getPayloadRange(offset, out, n);
      sendFrame(RSP_RANGE, out, n);
      break;
    }

    default:
      sendNack(0x7F);
      break;
  }
}

}  // namespace

void protocolInit(Stream& serial) {
  g_serial = &serial;
  resetParser();
}

void protocolProcess() {
  if (g_serial == nullptr) {
    return;
  }

  while (g_serial->available() > 0) {
    const uint8_t b = static_cast<uint8_t>(g_serial->read());

    switch (g_state) {
      case ParseState::WaitSof1:
        if (b == SOF1) g_state = ParseState::WaitSof2;
        break;

      case ParseState::WaitSof2:
        if (b == SOF2) {
          g_state = ParseState::WaitCmd;
        } else {
          resetParser();
        }
        break;

      case ParseState::WaitCmd:
        g_cmd = b;
        g_crc = b;
        g_state = ParseState::WaitLenL;
        break;

      case ParseState::WaitLenL:
        g_len = b;
        g_crc ^= b;
        g_state = ParseState::WaitLenH;
        break;

      case ParseState::WaitLenH:
        g_len |= static_cast<uint16_t>(b) << 8;
        g_crc ^= b;
        if (g_len > MAX_FRAME_PAYLOAD) {
          sendNack(0x05);
          resetParser();
        } else if (g_len == 0) {
          g_state = ParseState::WaitCrc;
        } else {
          g_pos = 0;
          g_state = ParseState::WaitPayload;
        }
        break;

      case ParseState::WaitPayload:
        g_payload[g_pos++] = b;
        g_crc ^= b;
        if (g_pos >= g_len) {
          g_state = ParseState::WaitCrc;
        }
        break;

      case ParseState::WaitCrc:
        if (b == g_crc) {
          handleFrame(g_cmd, g_payload, g_len);
        } else {
          sendNack(0x06);
        }
        resetParser();
        break;
    }
  }
}
