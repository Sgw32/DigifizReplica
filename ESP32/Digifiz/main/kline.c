// kline.c
#include "kline.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

#include "ecu_data.h"

#define TAG "KLINE"

static kline_state_t kline;

static const uint8_t cmd_start_comm[] = { 0x81, 0x10, 0xf1, 0x81, 0x03 };
static const uint8_t cmd_read_data[]  = { 0x82, 0x10, 0xF1, 0x21, 0x01, 0xA5 };
static const uint8_t cmd_read_dtcs[]  = { 0x84, 0x10, 0xF1, 0x18, 0x00, 0x00, 0x00, 0x9D };

void kline_task(void *pvParameters);

bool kline_parse_ecu_data(const uint8_t *data, size_t len, ecu_data_t *out) {
    if (len < 49) return false;

    // Checksum verification
    uint8_t sum = 0;
    for (int i = 6; i < 48; ++i) sum += data[i];
    if (data[48] != sum) {
        ESP_LOGW(TAG, "Checksum mismatch in ECU data");
        return false;
    }

    out->engine_temp_c       = (int8_t)(data[20] - 0x28);           // Temp C
    out->throttle_percent    = data[22];                            // %
    out->rpm = (out->throttle_percent == 0) 
               ? (uint16_t)(data[24] * 10)                          // Idle
               : (uint16_t)(data[23] * 40);                         // Load

    out->idle_regulator_pos  = data[26];                            // %
    out->battery_voltage     = 5.2f + (float)data[30] / 20.0f;      // V
    out->injector_duration_ms= ((data[35] << 8) | data[34]) / 125.0f; // ms
    out->air_mass_kgph       = ((data[37] << 8) | data[36]) / 10.0f; // kg/h
    out->lambda_voltage      = 1.25f * ((float)data[32] / 256.0f);   // V

    return true;
}

static uint8_t calc_checksum(const uint8_t *data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += data[i];
    }
    return sum;
}

void initKline(void) {
    uart_config_t config = {
        .baud_rate = KLINE_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(KLINE_UART_NUM, &config);
    uart_set_pin(KLINE_UART_NUM, KLINE_TX_PIN, KLINE_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(KLINE_UART_NUM, KLINE_BUFFER_SIZE * 2, 0, 0, NULL, 0);

    memset(&kline, 0, sizeof(kline));
    kline.mode = KLINE_MODE_CONNECTING;
    ESP_LOGI(TAG, "K-Line initialized on UART%d", KLINE_UART_NUM);
    xTaskCreatePinnedToCore(kline_task, "kline_task", 4096, NULL, 1, NULL, 1);
}

void kline_send(const uint8_t *data, uint8_t len) {
    uart_write_bytes(KLINE_UART_NUM, (const char *)data, len);
    ESP_LOGI(TAG, "Sent %d bytes", len);
}

static void kline_request_next(void) {
    const uint8_t *cmd = NULL;
    size_t len = 0;

    switch (kline.mode) {
        case KLINE_MODE_CONNECTING:
            cmd = cmd_start_comm; len = sizeof(cmd_start_comm); break;
        case KLINE_MODE_READDATA:
            cmd = cmd_read_data; len = sizeof(cmd_read_data); break;
        case KLINE_MODE_READERROR:
            cmd = cmd_read_dtcs; len = sizeof(cmd_read_dtcs); break;
        default:
            return;
    }

    kline_send(cmd, len);
}

void kline_handle_response(void) {
    int len = uart_read_bytes(KLINE_UART_NUM, kline.rx_msg.data, KLINE_BUFFER_SIZE, 20 / portTICK_PERIOD_MS);
    if (len < 1) return;

    kline.rx_msg.len = len;
    ESP_LOGI(TAG, "Received %d bytes", len);

    if (len < 5) {
        ESP_LOGW(TAG, "Response too short");
        return;
    }

    uint8_t checksum = calc_checksum(kline.rx_msg.data + len - 1, 1);  // last byte
    if (kline.rx_msg.data[len - 1] != calc_checksum(kline.rx_msg.data, len - 1)) {
        ESP_LOGW(TAG, "Checksum mismatch");
        return;
    }

    switch (kline.mode) {
        case KLINE_MODE_CONNECTING:
            if (kline.rx_msg.data[8] == 0xC1) {
                kline.mode = KLINE_MODE_READDATA;
                ESP_LOGI(TAG, "ECU connected, switching to read mode");
            }
            break;

        case KLINE_MODE_READDATA:
            ecu_data_t ecu;
            if (kline_parse_ecu_data(kline.rx_msg.data, kline.rx_msg.len, &ecu)) {
                ESP_LOGI(TAG, "Temp: %d°C, Throttle: %u%%, RPM: %u",
                            ecu.engine_temp_c, ecu.throttle_percent, ecu.rpm);
                ESP_LOGI(TAG, "Idle Reg: %u, Voltage: %.2fV, Inj: %.2fms",
                            ecu.idle_regulator_pos, ecu.battery_voltage, ecu.injector_duration_ms);
                ESP_LOGI(TAG, "Air: %.1f kg/h, Lambda: %.3f V",
                            ecu.air_mass_kgph, ecu.lambda_voltage);
            }
            break;

        case KLINE_MODE_READERROR:
            // handle DTC parsing
            break;

        default: break;
    }
}

void kline_task(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(250);  // 250 ms delay
    TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize the wake time

    while (1) {
        kline_request_next();         // Send next K-line request
        kline_handle_response();      // Handle the response

        vTaskDelayUntil(&xLastWakeTime, xDelay); // Delay until the next cycle
    }
}

