// kline.h
#ifndef KLINE_H
#define KLINE_H

#include <stdint.h>
#include <stdbool.h>

#define KLINE_UART_NUM      UART_NUM_1
#define KLINE_TX_PIN        43
#define KLINE_RX_PIN        44
#define KLINE_BAUD_RATE     10400
#define KLINE_BUFFER_SIZE   128

typedef enum {
    KLINE_MODE_DISCONNECTED = 0,
    KLINE_MODE_CONNECTING,
    KLINE_MODE_READDATA,
    KLINE_MODE_READERROR,
    KLINE_MODE_SPEEDSAMPLE
} kline_mode_t;

typedef struct {
    uint8_t data[KLINE_BUFFER_SIZE];
    uint16_t len;
} kline_message_t;

typedef struct {
    kline_mode_t mode;
    bool ready_to_send;
    uint32_t last_send_time_ms;
    kline_message_t rx_msg;
} kline_state_t;

void initKline(void);
void kline_send(const uint8_t *data, uint8_t len);
void kline_handle_response(void);

#endif // KLINE_H