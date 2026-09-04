// kline.h
#ifndef KLINE_H
#define KLINE_H

/**
 * @file kline.h
 * @brief K-Line (ISO 9141) communication interface for ECU data.
 */

#include <stdint.h>
#include <stdbool.h>

/** @brief UART peripheral used for K-Line communication. */
#define KLINE_UART_NUM      UART_NUM_1
/** @brief GPIO pin used as K-Line TX output. */
#define KLINE_TX_PIN        43
/** @brief GPIO pin used as K-Line RX input. */
#define KLINE_RX_PIN        44
/** @brief K-Line bus baud rate (ISO 9141 typical). */
#define KLINE_BAUD_RATE     10400
/** @brief Maximum received K-Line message payload length. */
#define KLINE_BUFFER_SIZE   128

/**
 * @brief K-Line protocol connection and polling state.
 */
typedef enum {
    /** @brief Link is not initialized or disconnected. */
    KLINE_MODE_DISCONNECTED = 0,
    /** @brief Connection handshake is in progress. */
    KLINE_MODE_CONNECTING,
    /** @brief Reading standard data frame sequence. */
    KLINE_MODE_READDATA,
    /** @brief Reading ECU error frame sequence. */
    KLINE_MODE_READERROR,
    /** @brief Reading speed sample frame sequence. */
    KLINE_MODE_SPEEDSAMPLE
} kline_mode_t;

/**
 * @brief Raw K-Line frame container.
 */
typedef struct {
    /** @brief Byte buffer with K-Line message payload. */
    uint8_t data[KLINE_BUFFER_SIZE];
    /** @brief Number of valid bytes stored in @ref data. */
    uint16_t len;
} kline_message_t;

/**
 * @brief Runtime K-Line protocol state.
 */
typedef struct {
    /** @brief Current K-Line state machine mode. */
    kline_mode_t mode;
    /** @brief Indicates transmitter may send a new frame. */
    bool ready_to_send;
    /** @brief Timestamp of the last transmitted frame (ms). */
    uint32_t last_send_time_ms;
    /** @brief Last received message buffer and length. */
    kline_message_t rx_msg;
} kline_state_t;

/**
 * @brief Initialize K-Line UART and communication state machine.
 */
void initKline(void);

/**
 * @brief Send raw K-Line frame bytes.
 *
 * @param data Pointer to frame payload.
 * @param len Number of bytes to transmit.
 */
void kline_send(const uint8_t *data, uint8_t len);

/**
 * @brief Poll and process incoming K-Line response data.
 */
void kline_handle_response(void);

#endif // KLINE_H
