#include "reg_inout.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_rom_sys.h"
#include <stdlib.h>

static const char TAG[] = "reg_inout";

#define REG_INOUT_HOST       SPI2_HOST
#define PIN_NUM_MISO         13      // 74HC165 QH / DATA
#define PIN_NUM_MOSI         11      // currently unused; 74HC595 omitted
#define PIN_NUM_CLK          12      // 74HC165 CP / CLOCK, idle LOW
#define PIN_NUM_CS_OUT       40      // currently unused; 74HC595 omitted
#define PIN_NUM_CS_IN        37      // 74HC165 /PL / LATCH, idle HIGH

DigifizOut digifiz_reg_out;
DigifizIn digifiz_reg_in;

#ifdef DIGIFIZ_NEXT_DISPLAY

spi_bus_config_t buscfg = {
    .miso_io_num = PIN_NUM_MISO,
    .mosi_io_num = PIN_NUM_MOSI,
    .sclk_io_num = PIN_NUM_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 32,
};

hc595_config_t hc595_config = {
    .cs_io = PIN_NUM_CS_OUT,
    .host = REG_INOUT_HOST,
};

hc165_config_t hc165_config = {
    .cs_io = PIN_NUM_CS_IN,
    .host = REG_INOUT_HOST,
};

hc595_handle_t hc595_handle;
hc165_handle_t hc165_handle;

static void hc595_cs_high(spi_transaction_t* t)
{
    ESP_EARLY_LOGV(TAG, "hc595 cs high %d.", ((hc595_context_t*)t->user)->cfg.cs_io);
    gpio_set_level(((hc595_context_t*)t->user)->cfg.cs_io, 1);
}

static void hc595_cs_low(spi_transaction_t* t)
{
    gpio_set_level(((hc595_context_t*)t->user)->cfg.cs_io, 0);
    ESP_EARLY_LOGV(TAG, "hc595 cs low %d.", ((hc595_context_t*)t->user)->cfg.cs_io);
}

static void hc165_cs_high(spi_transaction_t* t)
{
    gpio_set_level(((hc165_context_t*)t->user)->cfg.cs_io, 0);
    ESP_EARLY_LOGV(TAG, "hc165 cs low %d.", ((hc165_context_t*)t->user)->cfg.cs_io);
    vTaskDelay(pdMS_TO_TICKS(1));
    ESP_EARLY_LOGV(TAG, "hc165 cs high %d.", ((hc165_context_t*)t->user)->cfg.cs_io);
    gpio_set_level(((hc165_context_t*)t->user)->cfg.cs_io, 1);
}

static void hc165_cs_low(spi_transaction_t* t)
{
    
}

// Function to initialize SPI
esp_err_t  spi_devices_init(const hc595_config_t *cfg_regout, hc595_handle_t* out_ctx_regout,
                                   const hc165_config_t *cfg_regin, hc165_handle_t* out_ctx_regin)
{
    esp_err_t err = ESP_OK;

    hc595_context_t* ctx_regout = (hc595_context_t*)malloc(sizeof(hc595_context_t));
    if (!ctx_regout) {
        return ESP_ERR_NO_MEM;
    }

    hc165_context_t* ctx_regin = (hc165_context_t*)malloc(sizeof(hc165_context_t));
    if (!ctx_regout) {
        return ESP_ERR_NO_MEM;
    }

    *ctx_regout = (hc595_context_t) {
        .cfg = *cfg_regout,
    };

    *ctx_regin = (hc165_context_t) {
        .cfg = *cfg_regin,
    };

    spi_device_interface_config_t devcfg_regout = {
        .command_bits = 8,
        .clock_speed_hz = SPI_CLK,
        .mode = 0,          //SPI mode 0
        .spics_io_num = -1,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .pre_cb = hc595_cs_low,
        .post_cb = hc595_cs_high,
        .input_delay_ns = HC595_INPUT_DELAY_NS, 
    };

    spi_device_interface_config_t devcfg_regin = {
        .command_bits = 0,
        .clock_speed_hz = SPI_CLK,
        .mode = 1,          
        .spics_io_num = -1,
        .queue_size = 1,
        .pre_cb = hc165_cs_high,
        .post_cb = hc165_cs_low,
        .input_delay_ns = HC165_INPUT_DELAY_NS,  
    };
    
    err = spi_bus_add_device(ctx_regout->cfg.host, &devcfg_regout, &ctx_regout->spi);
    if (err != ESP_OK) {
        goto cleanup;
    }

    err = spi_bus_add_device(ctx_regin->cfg.host, &devcfg_regin, &ctx_regin->spi);
    if (err != ESP_OK) {
        goto cleanup;
    }

    gpio_set_level(ctx_regout->cfg.cs_io, 1);
    gpio_config_t cs_cfg_out = {
        .pin_bit_mask = BIT64(ctx_regout->cfg.cs_io),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&cs_cfg_out);

    gpio_set_level(ctx_regin->cfg.cs_io, 1);
    gpio_config_t cs_cfg_in = {
        .pin_bit_mask = BIT64(ctx_regin->cfg.cs_io),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&cs_cfg_in);

    *out_ctx_regout = ctx_regout;
    *out_ctx_regin = ctx_regin;
    return ESP_OK;

cleanup:
    if (ctx_regout->spi) {
        spi_bus_remove_device(ctx_regout->spi);
        ctx_regout->spi = NULL;
    }
    if (ctx_regout->ready_sem) {
        vSemaphoreDelete(ctx_regout->ready_sem);
        ctx_regout->ready_sem = NULL;
    }

    if (ctx_regin->spi) {
        spi_bus_remove_device(ctx_regin->spi);
        ctx_regin->spi = NULL;
    }
    if (ctx_regin->ready_sem) {
        vSemaphoreDelete(ctx_regin->ready_sem);
        ctx_regin->ready_sem = NULL;
    }
    free(ctx_regout);
    free(ctx_regin);
    return err;
}

esp_err_t spi_hc165_read(hc165_context_t* ctx, uint8_t* out_data)
{
    esp_err_t err;
    err = spi_device_acquire_bus(ctx->spi, portMAX_DELAY);
    if (err != ESP_OK) {
        return err;
    }

    spi_transaction_t t = {
        .cmd = 0xFFFF,
        .length = 16,
        .rxlength = 16,
        .flags = SPI_TRANS_USE_RXDATA,
        .user = ctx,
    };
    err = spi_device_polling_transmit(ctx->spi, &t);
    if (err != ESP_OK) {
        return err;
    }

    *out_data = t.rx_data[1];
    *(out_data+1) = t.rx_data[0];
    // ESP_LOGI(TAG, "READ HC165 0 %u",t.rx_data[0]);
    // ESP_LOGI(TAG, "READ HC165 1 %u",t.rx_data[1]);
    spi_device_release_bus(ctx->spi);
    return ESP_OK;
}

esp_err_t spi_hc595_write(hc595_context_t* ctx, uint8_t data)
{
    // ESP_LOGI(TAG, "Write to HC595 %d",(int)data);
    esp_err_t err;
    err = spi_device_acquire_bus(ctx->spi, portMAX_DELAY);
    if (err != ESP_OK) {
        return err;
    }

    spi_transaction_t t = {
        .cmd = data,
        .length = 0,
        .flags = 0,
        .user = ctx,
    };
    err = spi_device_polling_transmit(ctx->spi, &t);

    spi_device_release_bus(ctx->spi);
    return err;
}

esp_err_t regin_read(uint8_t* out_data)
{
    return spi_hc165_read(hc165_handle, out_data);
}

esp_err_t regout_all(uint16_t data)
{
    return spi_hc595_write(hc595_handle, data);
}

void initRegInOut(void)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "initRegInOut started");
    ESP_LOGI(TAG, "Initializing bus SPI%d...", REG_INOUT_HOST + 1);

    gpio_set_level(41, 0);
    gpio_config_t exmosi_cfg_out = {
        .pin_bit_mask = BIT64(41),
        .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&exmosi_cfg_out);

    //Initialize the SPI bus
    ret = spi_bus_initialize(REG_INOUT_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Initializing device...");
    ret = spi_devices_init(&hc595_config, &hc595_handle, &hc165_config, &hc165_handle);
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "initRegInOut ended");
}
#endif

#ifdef DIGIFIZ_REFIZ_DISPLAY

#define HC165_COUNT          3
#define HC165_PULSE_WIDTH_US 10

hc595_config_t hc595_config = {
    .cs_io = PIN_NUM_CS_OUT,
    .host = REG_INOUT_HOST,
};

hc165_config_t hc165_config = {
    .cs_io = PIN_NUM_CS_IN,
    .host = REG_INOUT_HOST,
};

/*
 * 74HC595 operation is intentionally omitted for now.
 * The Arduino example reads 74HC165 by bit-banging, not by SPI.transfer().
 */
hc595_handle_t hc595_handle = NULL;
hc165_handle_t hc165_handle = NULL;

static inline void hc165_latch_inputs(void)
{
    // Arduino behavior:
    // digitalWrite(latchPin, LOW);
    // delayMicroseconds(pulseWidth);
    // digitalWrite(latchPin, HIGH);
    gpio_set_level(PIN_NUM_CS_IN, 0);
    esp_rom_delay_us(HC165_PULSE_WIDTH_US);
    gpio_set_level(PIN_NUM_CS_IN, 1);
    esp_rom_delay_us(HC165_PULSE_WIDTH_US);
}

static uint8_t hc165_read_one(void)
{
    uint8_t ret = 0;

    /*
     * Same behavior as the Arduino ReadOne165():
     * - clock idles LOW
     * - read the current bit first
     * - then pulse clock HIGH -> LOW
     * - first bit read is D7, so store from bit 7 down to bit 0
     */
    for (int i = 7; i >= 0; --i) {
        if (gpio_get_level(PIN_NUM_MISO)) {
            ret |= (uint8_t)(1U << i);
        }

        gpio_set_level(PIN_NUM_CLK, 1);
        esp_rom_delay_us(HC165_PULSE_WIDTH_US);
        gpio_set_level(PIN_NUM_CLK, 0);
        esp_rom_delay_us(HC165_PULSE_WIDTH_US);
    }

    return ret;
}

esp_err_t spi_devices_init(const hc595_config_t *cfg_regout, hc595_handle_t* out_ctx_regout,
                           const hc165_config_t *cfg_regin, hc165_handle_t* out_ctx_regin)
{
    (void)cfg_regout;
    hc165_context_t* ctx_regin = (hc165_context_t*)calloc(1, sizeof(hc165_context_t));
    if (!ctx_regin) {
        return ESP_ERR_NO_MEM;
    }

    ctx_regin->cfg = cfg_regin ? *cfg_regin : hc165_config;

    gpio_config_t hc165_gpio_cfg = {
        .pin_bit_mask = BIT64(PIN_NUM_CS_IN) | BIT64(PIN_NUM_CLK),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&hc165_gpio_cfg));

    gpio_config_t hc165_data_cfg = {
        .pin_bit_mask = BIT64(PIN_NUM_MISO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&hc165_data_cfg));

    // Match Arduino idle states: CLOCK LOW, LATCH HIGH.
    gpio_set_level(PIN_NUM_CLK, 0);
    gpio_set_level(PIN_NUM_CS_IN, 1);

    if (out_ctx_regout) {
        *out_ctx_regout = NULL;
    }
    if (out_ctx_regin) {
        *out_ctx_regin = ctx_regin;
    }

    return ESP_OK;
}

esp_err_t spi_hc165_read(hc165_context_t* ctx, uint8_t* out_data)
{
    (void)ctx;

    if (!out_data) {
        return ESP_ERR_INVALID_ARG;
    }

    hc165_latch_inputs();

    /*
     * Arduino example with 4 registers did:
     *   for (int i = 24; i >= 0; i -= 8)
     *       optionSwitch |= ReadOne165() << i;
     *
     * For 3 registers, keep the same ordering but only read 24 bits:
     * first byte read = highest byte, then middle, then lowest.
     */
    for (int i = 0; i < HC165_COUNT; ++i) {
        out_data[i] = hc165_read_one();
    }

    return ESP_OK;
}

esp_err_t spi_hc595_write(hc595_context_t* ctx, uint8_t data)
{
    (void)ctx;
    (void)data;

    // 74HC595 is intentionally omitted for now.
    return ESP_OK;
}

esp_err_t regin_read(uint8_t* out_data)
{
    return spi_hc165_read(hc165_handle, out_data);
}

esp_err_t regout_all(uint16_t data)
{
    (void)data;

    // 74HC595 is intentionally omitted for now.
    return ESP_OK;
}

void initRegInOut(void)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "initRegInOut started");

    ESP_LOGI(TAG, "Initializing HC165 GPIO bit-bang interface...");
    ret = spi_devices_init(NULL, &hc595_handle, &hc165_config, &hc165_handle);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "initRegInOut ended");
}


#endif
