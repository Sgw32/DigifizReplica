#ifndef REG_INOUT_H
#define REG_INOUT_H

/**
 * @file reg_inout.h
 * @brief Shift-register based GPIO input/output abstraction.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "hal/spi_types.h"
#include "esp_log.h"
#include <unistd.h>
#include <sys/param.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <stdint.h>

// Define your pin connections
#define HC165_SER_PIN  2  // Serial data input
#define HC165_Q7_PIN   3  // Serial data output
#define HC165_SRCLK_PIN 4 // Shift register clock input

// Define the GPIO pins connected to the 74HC595
#define HC595_SER_PIN  2  // GPIO pin connected to SER (DS)
#define HC595_SRCLK_PIN 3  // GPIO pin connected to SRCLK (SHCP)
#define HC595_RCLK_PIN  4  // GPIO pin connected to RCLK (STCP)

// Define the SPI bus and device configurations
#define SPI_BUS  SPI1_HOST
#define SPI_CLK  1000000 // SPI clock frequency (1 MHz)
#define SPI_MODE 0        // SPI mode (CPOL=0, CPHA=0)

#define HC595_BUSY_TIMEOUT_MS  5
#define HC165_BUSY_TIMEOUT_MS  5
#define HC595_INPUT_DELAY_NS 1000
#define HC165_INPUT_DELAY_NS 10000

// Define struct with union inside
typedef struct DigifizOut{
    union {
        struct {
            uint8_t byte;  // 8-bit variable
        };
        struct {
            uint8_t led_mfa1 : 1;  // 1-bit variable
            uint8_t led_mfa2 : 1;  // 1-bit variable
            uint8_t led_p_oil : 1; // 1-bit variable
            uint8_t buzzer : 1;    // 1-bit variable
            uint8_t : 4;            // Padding to ensure byte alignment
        };
    };
} DigifizOut;

typedef struct DigifizIn{
    union {
        struct {
            uint8_t bytes[2];  // 8-bit variable
        };
        struct {
            uint16_t dummy : 1;    // 1-bit variable
            uint16_t oil03 : 1;  // 1-bit variable
            uint16_t oil18 : 1;  // 1-bit variable
            uint16_t glheatInd : 1; // 1-bit variable
            uint16_t checkEng : 1;    // 1-bit variable
            uint16_t mfaMode : 1;    // 1-bit variable
            uint16_t mfaBlock : 1;    // 1-bit variable
            uint16_t mfaReset : 1;    // 1-bit variable
            uint16_t lightsInd : 1;    // 1-bit variable
            
            uint16_t lightsHeatInd : 1;    // 1-bit variable
            uint16_t fogLightsInd : 1;    // 1-bit variable
            uint16_t farLightsInd : 1;    // 1-bit variable
            uint16_t blinkLeftInd : 1;    // 1-bit variable
            uint16_t blinkRightInd : 1;    // 1-bit variable
            uint16_t brakesInd : 1;    // 1-bit variable
            uint16_t blinkAll : 1;    // 1-bit variable
        };
    };
} DigifizIn;

/// Configurations of the spi_hc595
typedef struct {
    spi_host_device_t host; ///< The SPI host used, set before calling `spi_hc595_init()`
    gpio_num_t cs_io;       ///< CS gpio number, set before calling `spi_hc595_init()`
} hc595_config_t;

/// Configurations of the spi_hc595
typedef struct {
    spi_host_device_t host; ///< The SPI host used, set before calling `spi_hc595_init()`
    gpio_num_t cs_io;       ///< CS gpio number, set before calling `spi_hc595_init()`
} hc165_config_t;

struct hc595_context_t{
    hc595_config_t cfg;        ///< Configuration by the caller.
    spi_device_handle_t spi;    ///< SPI device handle
    SemaphoreHandle_t ready_sem; ///< Semaphore for ready signal
};

struct hc165_context_t {
    hc165_config_t cfg;        ///< Configuration by the caller.
    spi_device_handle_t spi;    ///< SPI device handle
    SemaphoreHandle_t ready_sem; ///< Semaphore for ready signal
};

typedef struct hc595_context_t hc595_context_t;
typedef struct hc165_context_t hc165_context_t;

typedef struct hc595_context_t* hc595_handle_t;
typedef struct hc165_context_t* hc165_handle_t;

/**
 * @brief Initialize register input/output subsystem.
 */
void initRegInOut(void);

/**
 * @brief Initialize SPI devices for 74HC595 output and 74HC165 input chips.
 *
 * @param cfg_regout Configuration for HC595 output device.
 * @param out_ctx_regout Returned HC595 context handle.
 * @param cfg_regin Configuration for HC165 input device.
 * @param out_ctx_regin Returned HC165 context handle.
 * @return esp_err_t ESP_OK on success or an error code.
 */
esp_err_t spi_devices_init(const hc595_config_t *cfg_regout, hc595_handle_t* out_ctx_regout,
                             const hc165_config_t *cfg_regin, hc165_handle_t* out_ctx_regin);

/**
 * @brief Read a byte from HC165 shift register.
 *
 * @param handle Initialized HC165 handle.
 * @param out_data Destination byte pointer.
 * @return esp_err_t ESP_OK on success or an error code.
 */
esp_err_t spi_hc165_read(hc165_handle_t handle, uint8_t* out_data);

/**
 * @brief Write a byte to HC595 shift register.
 *
 * @param handle Initialized HC595 handle.
 * @param data Byte to shift out.
 * @return esp_err_t ESP_OK on success or an error code.
 */
esp_err_t spi_hc595_write(hc595_handle_t handle, uint8_t data);

/**
 * @brief Read current state from digital input register chain.
 *
 * @param out_data Pointer to output byte buffer.
 * @return esp_err_t ESP_OK on success or an error code.
 */
esp_err_t regin_read(uint8_t* out_data);

/**
 * @brief Write all digital outputs in one operation.
 *
 * @param data Bitfield containing output states.
 * @return esp_err_t ESP_OK on success or an error code.
 */
esp_err_t regout_all(uint16_t data);

extern DigifizOut digifiz_reg_out;
extern DigifizIn digifiz_reg_in;

#endif /* REG_INOUT_H */
