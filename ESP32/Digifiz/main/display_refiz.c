#include "display_next.h"
#ifdef DIGIFIZ_REFIZ_DISPLAY
#include "reg_inout.h"
#include "esp_log.h"
//#include "mjs.h"
#include <cJSON.h>
//If updating, do not display anything
#include "digifiz_ws_server.h"
#include "params.h"
#include "led_effects.h"
#include "millis.h"
#include "crc8.h"
#include "driver/uart.h"
#include "esp_err.h"

#define TAG "display_refiz"
#define SCRIPT_KEY "display_refiz_script"

uint8_t selectedBrightness = 20;
uint32_t mRPMData = 4000;
uint8_t backlightOff = 0;

uint8_t backlightLevel = 30;

DigifizNextDisplay display;
static led_strip_handle_t led_strip;
float brightnessFiltered = 6.0f;
static led_effect_state_t effect_state;

static uint16_t l_spd_m = 0;
static uint16_t last_coolant_value = 0;

static void refresh_speed_digit_colors(uint16_t speed_value);
static void apply_speed_digit_override_color(uint8_t r, uint8_t g, uint8_t b);

typedef struct
{
    uint8_t output;
    uint8_t pending;
    uint16_t stable_count;
    bool initialized;
} indicator_filter_t;

typedef struct
{
    uint32_t last_active_ms;
    bool seen;
} turn_signal_latch_t;

static indicator_filter_t brakes_indicator_filter;
static indicator_filter_t foglight1_indicator_filter;
static indicator_filter_t foglight2_indicator_filter;
static indicator_filter_t glassheat_indicator_filter;
static indicator_filter_t left_turn_indicator_filter;
static indicator_filter_t right_turn_indicator_filter;
static turn_signal_latch_t left_turn_latch;
static turn_signal_latch_t right_turn_latch;

#define TURN_BLINK_PERIOD_MS 500
#define TURN_BLINK_INPUT_HOLD_MS 600
#define CHECK_ENGINE_BLINK_PERIOD_MS 1000
#define CHECK_ENGINE_ACTION_NONE 0
#define CHECK_ENGINE_ACTION_TEMP_BLINK 1
#define CHECK_ENGINE_ACTION_SPEED_BLINK 2

static uint8_t turn_blink_state = 0;
static uint32_t turn_blink_last_toggle_ms = 0;
static bool turn_blink_initialized = false;
static bool check_engine_active = false;
static uint32_t check_engine_last_toggle_ms = 0;
static bool check_engine_blink_state = true;
static bool check_engine_speed_override_active = false;

static uint8_t d_clock_hours = 0;
static uint8_t d_clock_minutes = 0;

#define PAYLOAD_SIZE 520
#define PACKED_PAYLOAD_SIZE ((PAYLOAD_SIZE + 7) / 8)
//#define SEND_PAYLOAD_BITS 256 //we do not transmit extra data
#define REFIZ_UART_BATCH_OFFSET_START 0
#define REFIZ_UART_BATCH_BYTES 33
#define REFIZ_UART_BATCH_BITS (REFIZ_UART_BATCH_BYTES * 8)

#define REFIZ_UART_BAUD 57600
#define REFIZ_UART_NUM UART_NUM_1
#define REFIZ_UART_TX_PIN 43
#define REFIZ_UART_RX_PIN UART_PIN_NO_CHANGE
#define REFIZ_UART_SET_RANGE_CMD 0x02
#define REFIZ_UART_SET_PACKED_RANGE_CMD 0x03
#define REFIZ_UART_SOF1 0xA5
#define REFIZ_UART_SOF2 0x5A
#define REFIZ_UART_TX_BUFFER_SIZE 1024

static const uint8_t refiz_default_payload[PACKED_PAYLOAD_SIZE] = {
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x3B,
  0xFE, 0x06, 0x00, 0x00, 0x00, 0xF8, 0xFB, 0xEF, 0xFD, 0x06, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8
};

static uint8_t bool_data_payload[PACKED_PAYLOAD_SIZE];
static bool refiz_uart_ready = false;

static const uint8_t refiz_orig_digit_patterns[10] = {
    0b00111111, 0b00100001, 0b01011011, 0b01110011, 0b01100101,
    0b01110110, 0b01111110, 0b00100011, 0b01111111, 0b01110111
};
static const uint16_t refiz_rpm_segments[71] = {
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    24, 25, 26, 27, 28, 29, 30, 31, 18, 17,
    159, 158, 157, 156, 155, 154, 153, 152, 151, 150,
    149, 148, 147, 146, 145, 253, 177, 178, 179, 180,
    181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
    191, 49, 50, 63, 62, 61, 60, 59, 58, 57,
    56, 105, 104, 103, 102, 101, 100, 99, 98, 97,
    96
};

static uint8_t refiz_payload_get_bit(uint16_t bit_index)
{
    if (bit_index >= PAYLOAD_SIZE)
    {
        return 0;
    }
    return (bool_data_payload[bit_index >> 3] & (1 << (bit_index & 0x07))) ? 1 : 0;
}

static void refiz_payload_set_bit(uint16_t bit_index, uint8_t value)
{
    if (bit_index >= PAYLOAD_SIZE)
    {
        return;
    }
    const uint8_t mask = 1 << (bit_index & 0x07);
    if (value)
    {
        bool_data_payload[bit_index >> 3] |= mask;
    }
    else
    {
        bool_data_payload[bit_index >> 3] &= ~mask;
    }
}

static uint8_t refiz_next_digit_to_orig(uint8_t next_pattern)
{
    static const uint8_t next_patterns[10] = {
        DIGIT_NUMBER_0, DIGIT_NUMBER_1, DIGIT_NUMBER_2, DIGIT_NUMBER_3, DIGIT_NUMBER_4,
        DIGIT_NUMBER_5, DIGIT_NUMBER_6, DIGIT_NUMBER_7, DIGIT_NUMBER_8, DIGIT_NUMBER_9
    };

    if (next_pattern == DIGIT_NUMBER_EMPTY)
    {
        return 0;
    }
    if (next_pattern == DIGIT_NUMBER_MINUS)
    {
        return 0b01000000;
    }
    for (uint8_t i = 0; i < 10; i++)
    {
        if (next_pattern == next_patterns[i])
        {
            return refiz_orig_digit_patterns[i];
        }
    }
    return 0;
}

static void refiz_put_digit(const uint16_t segments[7], uint8_t next_pattern)
{
    uint8_t orig_pattern = refiz_next_digit_to_orig(next_pattern);
    for (uint8_t i = 0; i < 7; i++)
    {
        refiz_payload_set_bit(segments[i] + 3, (orig_pattern & (1 << i)) ? 1 : 0);
    }
}

static void refiz_sync_payload_from_display(void)
{
    static const uint16_t mileage_segments[6][7] = {
        {75, 74, 74, 74, 74, 107, 106},
        {79, 76, 77, 109, 110, 111, 78},
        {83, 80, 81, 113, 114, 115, 82},
        {87, 84, 85, 117, 118, 119, 86},
        {91, 88, 89, 121, 122, 123, 90},
        {95, 92, 93, 125, 126, 127, 94}
    };
    static const uint16_t clock_segments[4][7] = {
        {3, 0, 1, 33, 34, 35, 2},
        {7, 4, 5, 37, 38, 39, 6},
        {11, 8, 9, 41, 42, 43, 10},
        {15, 12, 13, 45, 46, 47, 14}
    };
    static const uint16_t mfa_segments[4][7] = {
        {195, 192, 193, 225, 226, 227, 194},
        {199, 196, 197, 229, 230, 231, 198},
        {204, 201, 202, 234, 235, 236, 203},
        {208, 205, 206, 238, 239, 240, 207}
    };
    static const uint16_t fuel_segments[2][7] = {
        {129, 222, 223, 255, 160, 161, 128},
        {133, 130, 131, 163, 164, 165, 132}
    };
    static const uint16_t speed_segments[3][7] = {
        {252, 221, 221, 221, 221, 220, 221},
        {250, 249, 248, 218, 219, 216, 217},
        {244, 245, 246, 214, 215, 212, 213}
    };
    static const uint16_t coolant_segments[20] = {
        143, 142, 141, 140, 139, 138, 137, 136, 135, 134,
        166, 167, 168, 169, 170, 171, 172, 173, 174, 175
    };

    memcpy(bool_data_payload, refiz_default_payload, sizeof(bool_data_payload));

    refiz_put_digit(mileage_segments[0], display.mileage_digit_1);
    refiz_put_digit(mileage_segments[1], display.mileage_digit_2);
    refiz_put_digit(mileage_segments[2], display.mileage_digit_3);
    refiz_put_digit(mileage_segments[3], display.mileage_digit_4);
    refiz_put_digit(mileage_segments[4], display.mileage_digit_5);
    refiz_put_digit(mileage_segments[5], display.mileage_digit_6);

    refiz_put_digit(clock_segments[0], display.clock_digit_1);
    refiz_put_digit(clock_segments[1], display.clock_digit_2);
    refiz_put_digit(clock_segments[2], display.clock_digit_3);
    refiz_put_digit(clock_segments[3], display.clock_digit_4);
    refiz_payload_set_bit(40 + 3, (display.clock_dot != 0) ? 1 : 0);

    refiz_payload_set_bit(51 + 3, (display.mfa_indicators & 0x01) ? 1 : 0);
    refiz_payload_set_bit(19 + 3, (display.mfa_indicators & 0x02) ? 1 : 0);
    refiz_payload_set_bit(52 + 3, (display.mfa_indicators & 0x04) ? 1 : 0);
    refiz_payload_set_bit(20 + 3, (display.mfa_indicators & 0x10) ? 1 : 0);
    refiz_payload_set_bit(21 + 3, (display.mfa_indicators & 0x20) ? 1 : 0);
    refiz_payload_set_bit(53 + 3, (display.mfa_indicators & 0x40) ? 1 : 0);

    refiz_put_digit(mfa_segments[0], display.mfa_digit_1);
    refiz_put_digit(mfa_segments[1], display.mfa_digit_2);
    refiz_put_digit(mfa_segments[2], display.mfa_digit_3);
    refiz_put_digit(mfa_segments[3], display.mfa_digit_4);
    refiz_payload_set_bit(200 + 3, (display.mfa_dots & 0x01) ? 1 : 0);
    refiz_payload_set_bit(232 + 3, (display.mfa_dots & 0x04) ? 1 : 0);

    refiz_put_digit(fuel_segments[0], display.fuel_digit_1);
    refiz_put_digit(fuel_segments[1], display.fuel_digit_2);
    refiz_payload_set_bit(176 + 3, display.fuel_low_ind ? 1 : 0);

    refiz_put_digit(speed_segments[0], display.speed_digit_1);
    refiz_put_digit(speed_segments[1], display.speed_digit_2);
    refiz_put_digit(speed_segments[2], display.speed_digit_3);

    uint16_t coolant_value = display.coolant_value;
    for (uint8_t i = 0; i < 20; i++)
    {
        refiz_payload_set_bit(coolant_segments[i] + 3, (coolant_value & (1 << i)) ? 1 : 0);
    }

    const uint8_t *display_bits = (const uint8_t *)&display;
    uint8_t rpm_segments_on = 0;
    for (uint8_t bit = 28; bit < 78; bit++)
    {
        if (display_bits[bit / 8] & (1 << (bit % 8)))
        {
            rpm_segments_on++;
        }
    }
    for (uint8_t i = 0; i < 71; i++)
    {
        refiz_payload_set_bit(refiz_rpm_segments[i] + 3, (i < rpm_segments_on) ? 1 : 0);
    }
    refiz_payload_set_bit(refiz_rpm_segments[0] + 3, 1);
}

void refiz_uart_sender_init(void)
{
    if (refiz_uart_ready)
    {
        return;
    }

    uart_config_t uart_config = {
        .baud_rate = REFIZ_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(REFIZ_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(REFIZ_UART_NUM, REFIZ_UART_TX_PIN, REFIZ_UART_RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(REFIZ_UART_NUM, (REFIZ_UART_TX_BUFFER_SIZE*2), 0, 0, NULL, 0));
    memcpy(bool_data_payload, refiz_default_payload, sizeof(bool_data_payload));
    refiz_uart_ready = true;
    ESP_LOGI(TAG, "Refiz UART sender initialized on UART%d TX GPIO%d at %d baud",
             REFIZ_UART_NUM, REFIZ_UART_TX_PIN, REFIZ_UART_BAUD);
}

void refiz_uart_sender_trigger(void)
{
    if (!refiz_uart_ready)
    {
        return;
    }

    refiz_sync_payload_from_display();

    const uint16_t offset = REFIZ_UART_BATCH_OFFSET_START;
    const uint16_t bit_count = REFIZ_UART_BATCH_BITS;
    const uint16_t byte_count = REFIZ_UART_BATCH_BYTES;
    const uint16_t payload_len = 4 + byte_count;

    uint8_t frame[2 + 3 + 4 + REFIZ_UART_BATCH_BYTES + 1];

    size_t pos = 0;

    frame[pos++] = REFIZ_UART_SOF1;
    frame[pos++] = REFIZ_UART_SOF2;
    frame[pos++] = REFIZ_UART_SET_PACKED_RANGE_CMD;

    frame[pos++] = payload_len & 0xFF;
    frame[pos++] = (payload_len >> 8) & 0xFF;

    frame[pos++] = offset & 0xFF;
    frame[pos++] = (offset >> 8) & 0xFF;

    frame[pos++] = bit_count & 0xFF;
    frame[pos++] = (bit_count >> 8) & 0xFF;

    memcpy(&frame[pos], bool_data_payload, REFIZ_UART_BATCH_BYTES);
    pos += REFIZ_UART_BATCH_BYTES;

    frame[pos++] = crc8(&frame[2], pos - 2);
    /* Send in chunks */
    size_t tx_pos = 0;

    while (tx_pos < pos)
    {
        size_t chunk = pos - tx_pos;

        if (chunk > 8)
        {
            chunk = 8;
        }

        uart_write_bytes(
            REFIZ_UART_NUM,
            (const char *)&frame[tx_pos],
            chunk
        );

        tx_pos += chunk;

        /* Optional pause between chunks */
        //vTaskDelay(pdMS_TO_TICKS(2));
    }
}


static uint8_t apply_indicator_filter(indicator_filter_t *filter, uint8_t new_value, uint16_t required_cycles)
{
    uint8_t sanitized_value = new_value ? 1 : 0;

    if (!filter->initialized)
    {
        filter->output = sanitized_value;
        filter->pending = sanitized_value;
        filter->stable_count = 0;
        filter->initialized = true;
        return sanitized_value;
    }

    if (sanitized_value == filter->output)
    {
        filter->pending = sanitized_value;
        filter->stable_count = 0;
        return filter->output;
    }

    if (sanitized_value == filter->pending)
    {
        if (filter->stable_count < UINT16_MAX)
        {
            filter->stable_count++;
        }
    }
    else
    {
        filter->pending = sanitized_value;
        filter->stable_count = 1;
    }

    if ((required_cycles == 0) || (filter->stable_count >= required_cycles))
    {
        filter->output = filter->pending;
        filter->stable_count = 0;
    }

    return filter->output;
}

static uint8_t filter_general_indicator(indicator_filter_t *filter, uint8_t new_value)
{
    return apply_indicator_filter(filter, new_value, (uint16_t)digifiz_parameters.signalOptions_indicator_filter_cycles.value);
}

static uint8_t filter_turn_indicator(indicator_filter_t *filter, uint8_t new_value)
{
    return apply_indicator_filter(filter, new_value, (uint16_t)digifiz_parameters.signalOptions_turn_filter_cycles.value);
}

static void reset_indicator_filter(indicator_filter_t *filter, uint8_t value)
{
    uint8_t sanitized_value = value ? 1 : 0;
    filter->output = sanitized_value;
    filter->pending = sanitized_value;
    filter->stable_count = 0;
    filter->initialized = true;
}

static void reset_indicator_filters(void)
{
    reset_indicator_filter(&brakes_indicator_filter, display.brakes_ind);
    reset_indicator_filter(&foglight1_indicator_filter, display.foglight_ind1);
    reset_indicator_filter(&foglight2_indicator_filter, display.foglight_ind2);
    reset_indicator_filter(&glassheat_indicator_filter, display.glassheat_ind);
    reset_indicator_filter(&left_turn_indicator_filter, display.left_turn_ind);
    reset_indicator_filter(&right_turn_indicator_filter, display.right_turn_ind);
}

static void update_turn_blink_state(uint32_t now)
{
    if (!turn_blink_initialized)
    {
        turn_blink_state = 1;
        turn_blink_last_toggle_ms = now;
        turn_blink_initialized = true;
        return;
    }

    if ((now - turn_blink_last_toggle_ms) >= TURN_BLINK_PERIOD_MS)
    {
        turn_blink_state = turn_blink_state ? 0 : 1;
        turn_blink_last_toggle_ms = now;
    }
}

static void update_check_engine_blink_state(uint32_t now)
{
    if (!check_engine_last_toggle_ms)
    {
        check_engine_last_toggle_ms = now;
        check_engine_blink_state = true;
        return;
    }

    if ((now - check_engine_last_toggle_ms) >= CHECK_ENGINE_BLINK_PERIOD_MS)
    {
        check_engine_blink_state = !check_engine_blink_state;
        check_engine_last_toggle_ms = now;
    }
}

static uint8_t is_turn_signal_active(turn_signal_latch_t *latch, uint8_t raw_active, uint32_t now)
{
    if (raw_active)
    {
        latch->last_active_ms = now;
        latch->seen = true;
        return 1;
    }

    if (!latch->seen)
    {
        return 0;
    }

    return (now - latch->last_active_ms) <= TURN_BLINK_INPUT_HOLD_MS ? 1 : 0;
}

static uint8_t maincolor_r;
static uint8_t maincolor_g;
static uint8_t maincolor_b;
static uint8_t backcolor_r;
static uint8_t backcolor_g;
static uint8_t backcolor_b;

#define SPEED_DIGITS_FIRST_SEGMENT 234
#define SPEED_DIGITS_LAST_SEGMENT 269

static void restore_speed_digit_colors(void);
static void update_speed_digit_color_override(uint16_t speed_value);

static bool speed_alert_color_active = false;
int16_t redline_scheme_id = -1;

// Sample script (to be written to NVS initially)
const char *default_mjs_script =
    "function setColors() {"
    "  let arr = [];"
    "  for (let i = 0; i < 283; i++) {"
    "    arr.push({r: i % 256, g: (i * 2) % 256, b: (i * 3) % 256});"
    "  }"
    "  return arr;"
    "}";

ColoringScheme digifizCustom = {};

ColoringScheme digifizStandard = {
    .scheme = {
        { 
            .r = 20,
            .g = 20,
            .b = 2,
            .end_segment = 2,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 2
        },
        {
            .r = 20,
            .g = 20,
            .b = 2,
            .end_segment = 4,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 1
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 5,
            .type = COLOR_SCHEME_SIGNAL_MFA1_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 6,
            .type = COLOR_SCHEME_SIGNAL_MFA2_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 240,
            .g = 2,
            .b = 2,
            .end_segment = 7,
            .type = COLOR_SCHEME_SIGNAL_BATTERY_IND,
            .basecolor_enabled = 0
        },
        {
            .r = 20,
            .g = 20,
            .b = 2,
            .end_segment = 8,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 1
        },
        { 
            .r = 240,
            .g = 2,
            .b = 2,
            .end_segment = 9,
            .type = COLOR_SCHEME_SIGNAL_OIL_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 240,
            .g = 2,
            .b = 2,
            .end_segment = 10,
            .type = COLOR_SCHEME_SIGNAL_BRAKES_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 11,
            .type = COLOR_SCHEME_SIGNAL_RIGHT_TURN_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 12,
            .type = COLOR_SCHEME_SIGNAL_LEFT_TURN_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 2,
            .b = 120,
            .end_segment = 13,
            .type = COLOR_SCHEME_SIGNAL_FARLIGHT,
            .basecolor_enabled = 0
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 14,
            .type = COLOR_SCHEME_SIGNAL_GLASSHEAT,
            .basecolor_enabled = 0
        },
        {
            .r = 20,
            .g = 20,
            .b = 2,
            .end_segment = 15,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 1
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 16,
            .type = COLOR_SCHEME_SIGNAL_FOGLIGHTS2,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 17,
            .type = COLOR_SCHEME_SIGNAL_FOGLIGHTS1,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 80,
            .b = 2,
            .end_segment = 18,
            .type = COLOR_SCHEME_SIGNAL_LIGHTS,
            .basecolor_enabled = 0
        },
    }
};

#define REFUEL_SEG_TOP          (1 << 0)
#define REFUEL_SEG_TOP_LEFT     (1 << 1)
#define REFUEL_SEG_TOP_RIGHT    (1 << 2)
#define REFUEL_SEG_BOTTOM_LEFT  (1 << 3)
#define REFUEL_SEG_BOTTOM_RIGHT (1 << 4)
#define REFUEL_SEG_BOTTOM       (1 << 5)
#define REFUEL_PATTERN_R        0b111111

static const uint8_t refuel_digit_patterns[10] = {
    [0] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_LEFT | REFUEL_SEG_TOP_RIGHT |
          REFUEL_SEG_BOTTOM_LEFT | REFUEL_SEG_BOTTOM_RIGHT | REFUEL_SEG_BOTTOM,
    [1] = REFUEL_SEG_TOP_RIGHT | REFUEL_SEG_BOTTOM_RIGHT,
    [2] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_RIGHT | REFUEL_SEG_BOTTOM |
          REFUEL_SEG_BOTTOM_LEFT,
    [3] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_RIGHT | REFUEL_SEG_BOTTOM |
          REFUEL_SEG_BOTTOM_RIGHT,
    [4] = REFUEL_SEG_TOP_LEFT | REFUEL_SEG_TOP_RIGHT |
          REFUEL_SEG_BOTTOM_RIGHT,
    [5] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_LEFT | REFUEL_SEG_BOTTOM_RIGHT |
          REFUEL_SEG_BOTTOM,
    [6] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_LEFT | REFUEL_SEG_BOTTOM_LEFT |
          REFUEL_SEG_BOTTOM_RIGHT | REFUEL_SEG_BOTTOM,
    [7] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_RIGHT,
    [8] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_LEFT | REFUEL_SEG_TOP_RIGHT |
          REFUEL_SEG_BOTTOM_LEFT | REFUEL_SEG_BOTTOM_RIGHT | REFUEL_SEG_BOTTOM,
    [9] = REFUEL_SEG_TOP | REFUEL_SEG_TOP_LEFT | REFUEL_SEG_TOP_RIGHT |
          REFUEL_SEG_BOTTOM_RIGHT | REFUEL_SEG_BOTTOM,
};

static void set_refuel_pattern(uint8_t pattern)
{
    display.fuel_low_ind = pattern & 0x3F;
}

static void configure_led(void)
{
    ESP_LOGI(LOG_TAG, "Digifiz WS2812 LED init.");
     led_strip_config_t strip_config = {
        .strip_gpio_num = WSDATA_GPIO_PIN,
        .max_leds = DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS, // at least one LED on board
    };
    #if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void deinit_leds(void)
{
    ESP_LOGI(LOG_TAG, "Deinitializing WS2812 LED.");

    // Clear the LED strip to turn off all LEDs
    ESP_ERROR_CHECK(led_strip_clear(led_strip));

    // Delete the LED strip device
    ESP_ERROR_CHECK(led_strip_del(led_strip));
    led_strip = NULL;

    ESP_LOGI(LOG_TAG, "WS2812 LED deinitialized.");
}

// Initialize the display
void initDisplay() {
    ESP_LOGI(LOG_TAG, "initDisplay started");
    setBrightness(1);
    memset(&display, 0, sizeof(DigifizNextDisplay));
    display.battery_ind = 0;
    display.mfa1_ind = 0;
    display.mfa2_ind = 0;
    display.coolant_backlight = 1;
    display.vdo_backlight = 1;
    // display.clock_digit_1 = 0b1111111;
    setClockData(00,00);
    setMFADisplayedNumber(-99);
    setCoolantData(7);
    setRefuelSign(1);
    // display.mfa_digit_1 = DIGIT_NUMBER_8;
    // display.mfa_digit_2 = DIGIT_NUMBER_0;
    // display.mfa_dots = 0b111;
    // display.mfa_digit_3 = DIGIT_NUMBER_0;
    // display.mfa_digit_4 = DIGIT_NUMBER_0;
    display.clock_dot = 0b11;
    display.km_ind = 1;
    display.fuel_ind = 1;
    display.lights_on_ind = 0;
    display.foglight_ind2 = 0;
    display.backlight_leds = 0xFFFFFFFF;

    reset_indicator_filters();

    for (uint8_t i=0;i!=COLORING_SCHEME_MAX_ELEMENTS;i++)
    {
        if (digifizStandard.scheme[i].type==COLOR_SCHEME_RPM_REDLINE)
        {
            redline_scheme_id = i;
            break;
        }
    }
    //display.rpm_padding = 0xF;
    //display.rpm[0] = 0xFF;
    setFuel(99);
    configure_led();
    setMileage(123456);
    setRPMData(5);
    ESP_LOGI(LOG_TAG, "Digifiz WS2812 LED init OK.");
    //Compile colors from mJS backend
    //run_mjs_script(default_mjs_script);
    compileColorScheme();
    effect_state.effect = digifiz_parameters.ledEffect_type.value;
    effect_state.hue = digifiz_parameters.ledEffect_hue.value;
    effect_state.saturation = digifiz_parameters.ledEffect_saturation.value;
    effect_state.value = digifiz_parameters.ledEffect_value.value;
    led_effect_init(&effect_state);
    refiz_uart_sender_init();
    ESP_LOGI(LOG_TAG, "initDisplay ended");
}

// Set the RPM data
void setRPM(uint32_t rpmdata) {
    mRPMData=rpmdata;
}

// Set the clock data
void setClockData(uint8_t clock_hours, uint8_t clock_minutes) 
{
    d_clock_hours = clock_hours;
    d_clock_minutes = clock_minutes;
}

// Set the MFA clock data
void setMFAClockData(uint8_t mfa_clock_hours, uint8_t mfa_clock_minutes) {
    uint8_t number_clock[10]={DIGIT_NUMBER_0,
                            DIGIT_NUMBER_1,
                            DIGIT_NUMBER_2,
                            DIGIT_NUMBER_3,
                            DIGIT_NUMBER_4,
                            DIGIT_NUMBER_5,
                            DIGIT_NUMBER_6,
                            DIGIT_NUMBER_7,
                            DIGIT_NUMBER_8,
                            DIGIT_NUMBER_9};
    if (((mfa_clock_hours / 10) % 10)!=0)
    {
        display.mfa_digit_1 = number_clock[(mfa_clock_hours / 10) % 10];
        display.mfa_digit_2 = number_clock[(mfa_clock_hours / 1) % 10];
        display.mfa_digit_3 = number_clock[(mfa_clock_minutes / 10) % 10];
        display.mfa_digit_4 = number_clock[(mfa_clock_minutes / 1) % 10];
    }
    else
    {
        display.mfa_digit_1 = DIGIT_NUMBER_0;
        display.mfa_digit_2 = number_clock[(mfa_clock_hours / 1) % 10];
        display.mfa_digit_3 = number_clock[(mfa_clock_minutes / 10) % 10];
        display.mfa_digit_4 = number_clock[(mfa_clock_minutes / 1) % 10];
    }
}

// Set the MFA displayed number
void setMFADisplayedNumber(int16_t data) {
    uint8_t number_mfa[10]={DIGIT_NUMBER_0,
                            DIGIT_NUMBER_1,
                            DIGIT_NUMBER_2,
                            DIGIT_NUMBER_3,
                            DIGIT_NUMBER_4,
                            DIGIT_NUMBER_5,
                            DIGIT_NUMBER_6,
                            DIGIT_NUMBER_7,
                            DIGIT_NUMBER_8,
                            DIGIT_NUMBER_9};
    //printf("DisplayedMFA:%d\n",data);
    if (data>=0)
    {
      if (((data / 1000) % 10)!=0)
      {
        display.mfa_digit_1 = number_mfa[(data / 1000) % 10];
        display.mfa_digit_2 = number_mfa[(data / 100) % 10];
      }
      else
      {
        display.mfa_digit_1 = DIGIT_NUMBER_EMPTY;
        display.mfa_digit_2 = number_mfa[(data / 100) % 10];
      }
      display.mfa_digit_3 = number_mfa[(data / 10) % 10];
      display.mfa_digit_4 = number_mfa[(data / 1) % 10];
    }
    else
    {
      //minus values
      if (((-data / 1000) % 10)!=0)
      {
        display.mfa_digit_1 = number_mfa[(-data / 1000) % 10];
        display.mfa_digit_2 = number_mfa[(-data / 100) % 10];
      }
      else
      {
        if (((-data / 100) % 10)!=0)
        {
            display.mfa_digit_1 = DIGIT_NUMBER_MINUS;
            display.mfa_digit_2 = number_mfa[(-data / 100) % 10];
        }
        else
        {
            display.mfa_digit_1 = DIGIT_NUMBER_EMPTY;
            display.mfa_digit_2 = DIGIT_NUMBER_MINUS;
        }        
      }
      display.mfa_digit_3 = number_mfa[(-data / 10) % 10];
      display.mfa_digit_4 = number_mfa[(-data / 1) % 10];
    }
}

// Set the fuel level
void setFuel(uint8_t litres) {
    uint8_t number_fuel[10]={DIGIT_NUMBER_0,
        DIGIT_NUMBER_1,
        DIGIT_NUMBER_2,
        DIGIT_NUMBER_3,
        DIGIT_NUMBER_4,
        DIGIT_NUMBER_5,
        DIGIT_NUMBER_6,
        DIGIT_NUMBER_7,
        DIGIT_NUMBER_8,
        DIGIT_NUMBER_9};
    uint8_t data = litres;
    if (getFaultyMask().fuel_faulty==255)
    {
        display.fuel_digit_1 = DIGIT_NUMBER_MINUS;
        display.fuel_digit_2 = DIGIT_NUMBER_MINUS;
    }
    else
    {
        if (((data / 10) % 10)!=0)
        {
            display.fuel_digit_1 = number_fuel[(data / 10) % 10];
            display.fuel_digit_2 = number_fuel[(data / 1) % 10];
        }
        else
        {
            display.fuel_digit_1 = DIGIT_NUMBER_EMPTY;
            display.fuel_digit_2 = number_fuel[(data / 1) % 10];
        }  
    } 
}

// Set the RPM data
void setRPMData(uint16_t inp_d) {
    // Implementation placeholder
    if (inp_d>digifiz_parameters.maxRPM.value)
        inp_d = digifiz_parameters.maxRPM.value;
    uint32_t data = (49*(uint32_t)inp_d)/8000;
    if (data<1)
        data=1;
    if (data>49)
        data=49; 
    display.rpm_padding=0;
    memset(display.rpm,0,5); 
    display.rpm_last = 0;
    uint8_t *ptr = (uint8_t*)&display;
    for (uint16_t i = 28; i < 28+data; i++)
    {
        for (uint16_t j = i%8; j < 8; j++)
        {
            //uint8_t bit = (ptr[i] >> j) & 1;
            ptr[i/8]|=(1<<j);
        }
    }
    for (uint16_t i = 28+data; i < 28+49; i++)
    {
        for (uint16_t j = i%8; j < 8; j++)
        {
            ptr[i/8]&=~(1<<j);
        }
    }
    display.rpm_backlight = 0b111111111111111111;
}

// Set the speedometer data
void setSpeedometerData(uint16_t data) {
    l_spd_m = data;
    uint8_t number_spd[10]={DIGIT_NUMBER_0,
                            DIGIT_NUMBER_1,
                            DIGIT_NUMBER_2,
                            DIGIT_NUMBER_3,
                            DIGIT_NUMBER_4,
                            DIGIT_NUMBER_5,
                            DIGIT_NUMBER_6,
                            DIGIT_NUMBER_7,
                            DIGIT_NUMBER_8,
                            DIGIT_NUMBER_9};
    if (((data / 100) % 10)!=0)
    {
        display.speed_digit_1 = number_spd[(data / 100) % 10];
        display.speed_digit_2 = number_spd[(data / 10) % 10];
        display.speed_digit_3 = number_spd[(data / 1) % 10];
        display.speed_digit_s1 = number_spd[(data / 100) % 10];
        display.speed_digit_s2 = number_spd[(data / 10) % 10];
        display.speed_digit_s3 = number_spd[(data / 1) % 10];
    }
    else
    {
        if (((data / 10) % 10)!=0)
        {
            display.speed_digit_1 = DIGIT_NUMBER_EMPTY;
            display.speed_digit_2 = number_spd[(data / 10) % 10];
            display.speed_digit_3 = number_spd[(data / 1) % 10];
            display.speed_digit_s1 = DIGIT_NUMBER_EMPTY;
            display.speed_digit_s2 = number_spd[(data / 10) % 10];
            display.speed_digit_s3 = number_spd[(data / 1) % 10];
        }
        else
        {
            display.speed_digit_1 = DIGIT_NUMBER_EMPTY;
            display.speed_digit_2 = DIGIT_NUMBER_EMPTY;
            display.speed_digit_3 = number_spd[(data / 1) % 10];
            display.speed_digit_s1 = DIGIT_NUMBER_EMPTY;
            display.speed_digit_s2 = DIGIT_NUMBER_EMPTY;
            display.speed_digit_s3 = number_spd[(data / 1) % 10];
        }
    }

    update_speed_digit_color_override(data);
}

// Display current gear in format "-G-" where G is gear number or '-' for neutral
void setSpeedometerGear(int8_t gear) {
    uint8_t number_spd[10]={DIGIT_NUMBER_0,
                            DIGIT_NUMBER_1,
                            DIGIT_NUMBER_2,
                            DIGIT_NUMBER_3,
                            DIGIT_NUMBER_4,
                            DIGIT_NUMBER_5,
                            DIGIT_NUMBER_6,
                            DIGIT_NUMBER_7,
                            DIGIT_NUMBER_8,
                            DIGIT_NUMBER_9};
    display.speed_digit_1 = DIGIT_NUMBER_MINUS;
    display.speed_digit_s1 = DIGIT_NUMBER_MINUS;

    if (gear > 0 && gear < 10) {
        display.speed_digit_2 = number_spd[gear];
        display.speed_digit_s2 = number_spd[gear];
    } else {
        display.speed_digit_2 = DIGIT_NUMBER_MINUS;
        display.speed_digit_s2 = DIGIT_NUMBER_MINUS;
    }

    display.speed_digit_3 = DIGIT_NUMBER_MINUS;
    display.speed_digit_s3 = DIGIT_NUMBER_MINUS;

    if (speed_alert_color_active) {
        restore_speed_digit_colors();
    }
}

// Set the coolant temperature data
void setCoolantData(uint16_t data) {
    if (data>14)
        data=14; 
    display.coolant_value=0;
    for (uint8_t i=0;i<data;i++)
    {
        display.coolant_value|=(1<<i);
    }
    for (uint8_t i=data;i<14;i++)
    {
        display.coolant_value&=~(1<<i);
    }
    last_coolant_value = display.coolant_value;
}

// Set the dot status
void setDot(bool value) {
    // Implementation placeholder
}

// Set the floating dot status
void setFloatDot(bool value) {
    // Implementation placeholder
    if (value)
        display.mfa_dots |= 0b100;
    else
        display.mfa_dots &= ~0b100;
}

// Set the mileage data
void setMileage(uint32_t mileage) {
    uint8_t number[10]={DIGIT_NUMBER_0,
                            DIGIT_NUMBER_1,
                            DIGIT_NUMBER_2,
                            DIGIT_NUMBER_3,
                            DIGIT_NUMBER_4,
                            DIGIT_NUMBER_5,
                            DIGIT_NUMBER_6,
                            DIGIT_NUMBER_7,
                            DIGIT_NUMBER_8,
                            DIGIT_NUMBER_9};
  //We set mileage but do not set indicators
  if (((mileage / 100000) % 10)!=0)
  {
      display.mileage_digit_1 =  number[(mileage / 100000) % 10];
      display.mileage_digit_2 =  number[(mileage / 10000) % 10];
      display.mileage_digit_3 =  number[(mileage / 1000) % 10];
      display.mileage_digit_4 =  number[(mileage / 100) % 10];
      display.mileage_digit_5 =  number[(mileage / 10) % 10];
      display.mileage_digit_6 =  number[(mileage / 1) % 10];
  }
  else
  {
    if (((mileage / 10000) % 10)!=0)
    {
      display.mileage_digit_1 =  DIGIT_NUMBER_EMPTY;
      display.mileage_digit_2 =  number[(mileage / 10000) % 10];
      display.mileage_digit_3 =  number[(mileage / 1000) % 10];
      display.mileage_digit_4 =  number[(mileage / 100) % 10];
      display.mileage_digit_5 =  number[(mileage / 10) % 10];
      display.mileage_digit_6 =  number[(mileage / 1) % 10];
    }
    else
    {
      if (((mileage / 1000) % 10)!=0)
      {
        display.mileage_digit_1 =  DIGIT_NUMBER_EMPTY;
        display.mileage_digit_2 =  DIGIT_NUMBER_EMPTY;
        display.mileage_digit_3 =  number[(mileage / 1000) % 10];
        display.mileage_digit_4 =  number[(mileage / 100) % 10];
        display.mileage_digit_5 =  number[(mileage / 10) % 10];
        display.mileage_digit_6 =  number[(mileage / 1) % 10];
      }
      else
      {
        if (((mileage / 100) % 10)!=0)
        {
            display.mileage_digit_1 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_2 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_3 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_4 =  number[(mileage / 100) % 10];
            display.mileage_digit_5 =  number[(mileage / 10) % 10];
            display.mileage_digit_6 =  number[(mileage / 1) % 10];
        }
        else
        {
          if (((mileage / 10) % 10)!=0)
          {
            display.mileage_digit_1 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_2 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_3 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_4 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_5 =  number[(mileage / 10) % 10];
            display.mileage_digit_6 =  number[(mileage / 1) % 10];
          }
          else
          {
            display.mileage_digit_1 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_2 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_3 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_4 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_5 =  DIGIT_NUMBER_EMPTY;
            display.mileage_digit_6 =  number[(mileage / 1) % 10];
          }
        }
      }
    }
  }
}

// Set the MFA type
void setMFAType(uint8_t mfaType) {
    switch (mfaType)
    {
        case 0:
            display.mfa_indicators = 0b1;
            break;
        case 1:
            display.mfa_indicators = 0b10;
            break;
        case 2:
            display.mfa_indicators = 0b1100;
            break;
        case 3:
            display.mfa_indicators = 0b10000;
            break;
        case 4:
            display.mfa_indicators = 0b100000;
            break;
        case 5:
            display.mfa_indicators = 0b1000000;
            break;
        case 6:
            display.mfa_indicators = 0b1111111;
            break;
        default:
            break;
    }
    
}

extern int sinceStart_hours;
extern int sinceStart_minutes;

void displayMFAClock()
{
    uint8_t number_clock[10]={DIGIT_NUMBER_0,
                            DIGIT_NUMBER_1,
                            DIGIT_NUMBER_2,
                            DIGIT_NUMBER_3,
                            DIGIT_NUMBER_4,
                            DIGIT_NUMBER_5,
                            DIGIT_NUMBER_6,
                            DIGIT_NUMBER_7,
                            DIGIT_NUMBER_8,
                            DIGIT_NUMBER_9};
    if (digifiz_parameters.mfa_clock_sensor.value == MFA_CLOCK_SENSOR_CLOCK)
    {
        if (((d_clock_hours / 10) % 10)!=0)
        {
            display.clock_digit_1 = number_clock[(d_clock_hours / 10) % 10];
            display.clock_digit_2 = number_clock[(d_clock_hours / 1) % 10];
            display.clock_digit_3 = number_clock[(d_clock_minutes / 10) % 10];
            display.clock_digit_4 = number_clock[(d_clock_minutes / 1) % 10];
        }
        else
        {
            display.clock_digit_1 = DIGIT_NUMBER_0;
            display.clock_digit_2 = number_clock[(d_clock_hours / 1) % 10];
            display.clock_digit_3 = number_clock[(d_clock_minutes / 10) % 10];
            display.clock_digit_4 = number_clock[(d_clock_minutes / 1) % 10];
        }
        display.clock_dot = 0b11;
        return;
    }

    display.clock_dot = 0b10;
    int16_t sensorValue = getMFAClockSensorValue()*100.0f;
    if (sensorValue>=0)
    {
      if (((sensorValue / 1000) % 10)!=0)
      {
        display.clock_digit_1 = number_clock[(sensorValue / 1000) % 10];
        display.clock_digit_2 = number_clock[(sensorValue / 100) % 10];
      }
      else
      {
        display.clock_digit_1 = DIGIT_NUMBER_EMPTY;
        display.clock_digit_2 = number_clock[(sensorValue / 100) % 10];
      }
      display.clock_digit_3 = number_clock[(sensorValue / 10) % 10];
      display.clock_digit_4 = number_clock[(sensorValue / 1) % 10];
    }
    else
    {
      //minus values
      if (((-sensorValue / 1000) % 10)!=0)
      {
        display.clock_digit_1 = number_clock[(-sensorValue / 1000) % 10];
        display.clock_digit_2 = number_clock[(-sensorValue / 100) % 10];
      }
      else
      {
        if (((-sensorValue / 100) % 10)!=0)
        {
            display.clock_digit_1 = DIGIT_NUMBER_MINUS;
            display.clock_digit_2 = number_clock[(-sensorValue / 100) % 10];
        }
        else
        {
            display.clock_digit_1 = DIGIT_NUMBER_EMPTY;
            display.clock_digit_2 = DIGIT_NUMBER_MINUS;
        }        
      }
      display.clock_digit_3 = number_clock[(-sensorValue / 10) % 10];
      display.clock_digit_3 = number_clock[(-sensorValue / 1) % 10];
    }
    
}

// Display the MFA type
void displayMFAType(uint8_t mfaType) {
    switch(mfaType)
    {
        case MFA_STATE_TRIP_DURATION:
            display.mfa_dots|=0b11;
            setMFAClockData(sinceStart_hours,sinceStart_minutes);
            break;
        case MFA_STATE_TRIP_DISTANCE:
            display.mfa_dots&=0b00;
            setMFADisplayedNumber((uint16_t)(digifiz_status.daily_mileage[digifiz_parameters.mfaBlock.value]/3600));
            setFloatDot(false);
            break;
        case MFA_STATE_SENSOR:
            display.mfa_dots&=0b00;
            setMFADisplayedNumber((int16_t)(getMFASensorValue()*100.0f));
            setFloatDot(true);
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            display.mfa_dots&=0b00;
            setMFADisplayedNumber((uint16_t)fabs(digifiz_status.averageSpeed[digifiz_parameters.mfaBlock.value]));
            setFloatDot(false);
            break;
        case MFA_STATE_OIL_TEMP:
            display.mfa_dots&=0b00;
            if (getFaultyMask().oil_faulty)
            {
                setFloatDot(false);
                display.mfa_digit_1 = DIGIT_NUMBER_MINUS;
                display.mfa_digit_2 = DIGIT_NUMBER_MINUS;
            }
            else
            {
                if (digifiz_parameters.option_fahrenheit.value)
                {
                  setMFADisplayedNumber((int16_t)getOilTemperatureFahrenheit());
                  setFloatDot(false);
                }
                else if (digifiz_parameters.option_kelvin.value)
                {
                  setMFADisplayedNumber((int16_t)(getOilTemperature()+KELVIN_TO_CELSIUM));
                  setFloatDot(false);
                }
                else
                {
                  setMFADisplayedNumber((int16_t)(getOilTemperature()));
                  setFloatDot(false);
                }
            }
            break;
        case MFA_STATE_AIR_TEMP:
            display.mfa_dots&=0b00;
            if (getFaultyMask().air_faulty)
            {
                setFloatDot(false);
                display.mfa_digit_1 = DIGIT_NUMBER_MINUS;
                display.mfa_digit_2 = DIGIT_NUMBER_MINUS;
            }
            else
            {
                if (digifiz_parameters.option_fahrenheit.value)
                {
                setMFADisplayedNumber((int16_t)getAmbientTemperatureFahrenheit());
                setFloatDot(false);
                }
                else if (digifiz_parameters.option_kelvin.value)
                { 
                setMFADisplayedNumber((int16_t)(getAmbientTemperature()+KELVIN_TO_CELSIUM));
                setFloatDot(false);
                }
                else
                {
                setMFADisplayedNumber((int16_t)getAmbientTemperature());
                setFloatDot(false);
                }
            }
            break;
        case MFA_STATE_FUEL_PRESSURE:
            display.mfa_dots&=0b00;
            setMFADisplayedNumber((uint16_t)(getFuelPressure()*100.0f));
            setFloatDot(true);
            break;
        default:
            break;
    }
}

// Set the MFA block
void setMFABlock(uint8_t block) {
  if (digifiz_parameters.option_testmode_on.value)
  {  
    digifiz_reg_out.byte+=1;
    digifiz_reg_out.byte = digifiz_reg_out.byte%8;
  }
  else
  {
    if (block&0x1)
    {
      display.mfa1_ind = 1;
      display.mfa2_ind = 0;
      digifiz_reg_out.led_mfa1 = 1;
      digifiz_reg_out.led_mfa2 = 0;
    }
    else
    {
      display.mfa1_ind = 0;
      display.mfa2_ind = 1;
      digifiz_reg_out.led_mfa1 = 0;
      digifiz_reg_out.led_mfa2 = 1;
    }
  }
}

// Set the brightness level
void setBrightness(uint8_t levels) {
    float coef = ((float)digifiz_parameters.brightnessSpeed.value)/100.0f;
    brightnessFiltered += coef*((float)levels-brightnessFiltered);
    backlightLevel = (uint8_t)brightnessFiltered;
}

void resetBrightness()
{
    brightnessFiltered = 0.0f;
    backlightLevel = 0;
}

// Set the refuel sign status
void setRefuelSign(bool onoff) {
    set_refuel_pattern(onoff ? REFUEL_PATTERN_R : 0);
}

void setRefuelGear(int8_t gear) {
    if (gear > 0 && gear < 10) {
        set_refuel_pattern(refuel_digit_patterns[gear]);
    } else {
        set_refuel_pattern(0);
    }
}

// Set the check engine sign status
void setCheckEngine(bool onoff) {
    if (check_engine_active == onoff) {
        return;
    }

    check_engine_active = onoff;
    check_engine_last_toggle_ms = millis();
    check_engine_blink_state = true;
    if (!check_engine_active && check_engine_speed_override_active) {
        refresh_speed_digit_colors(l_spd_m);
        check_engine_speed_override_active = false;
    }
}

void applyCheckEngineAction(void)
{
    uint8_t action = digifiz_parameters.checkEngineAction.value;

    if (!check_engine_active || action == CHECK_ENGINE_ACTION_NONE)
    {
        if (check_engine_speed_override_active)
        {
            refresh_speed_digit_colors(l_spd_m);
            check_engine_speed_override_active = false;
        }
        return;
    }

    update_check_engine_blink_state(millis());

    if (action == CHECK_ENGINE_ACTION_TEMP_BLINK)
    {
        display.coolant_value = check_engine_blink_state ? last_coolant_value : 0;
        if (check_engine_speed_override_active)
        {
            refresh_speed_digit_colors(l_spd_m);
            check_engine_speed_override_active = false;
        }
    }
    else if (action == CHECK_ENGINE_ACTION_SPEED_BLINK)
    {
        if (check_engine_blink_state)
        {
            apply_speed_digit_override_color(255, 0, 0);
            check_engine_speed_override_active = true;
        }
        else
        {
            refresh_speed_digit_colors(l_spd_m);
            check_engine_speed_override_active = false;
        }
    }
    else if (check_engine_speed_override_active)
    {
        refresh_speed_digit_colors(l_spd_m);
        check_engine_speed_override_active = false;
    }
}

// Set the backlight status
void setBacklight(bool onoff) {
    backlightOff = onoff ? 1 : 0;
}

// Set the service display data
void setServiceDisplayData(uint8_t data) {
    
}

void setOilIndicator(bool onoff)
{
    display.oil_ind = onoff ? 1 : 0;
}
void setBrakesIndicator(bool onoff)
{
    display.brakes_ind = filter_general_indicator(&brakes_indicator_filter, onoff ? 1 : 0);
}
void setHeatLightsIndicator(bool onoff)
{
    display.foglight_ind1 = filter_general_indicator(&foglight1_indicator_filter, onoff ? 1 : 0);
}
void setBackLightsHeatIndicator(bool onoff)
{
    display.foglight_ind2 = filter_general_indicator(&foglight2_indicator_filter, onoff ? 1 : 0);
}
void setBackWindowHeatIndicator(bool onoff)
{
    display.glassheat_ind = filter_general_indicator(&glassheat_indicator_filter, onoff ? 1 : 0);
}
void processIndicators()
{
    uint8_t left_raw = 0;
    uint8_t right_raw = 0;

    if (digifiz_parameters.signalOptions_use_blink_alt_in.value)
    {
        left_raw = (!digifiz_reg_in.fogLightsInd) ? 1 : 0;
        if (!left_raw && !digifiz_reg_in.glheatInd)
        {
            right_raw = 1;
        }
    }
    else
    {
        // Normal way:
        if (digifiz_reg_in.blinkAll)
        {
            left_raw = (!digifiz_reg_in.blinkLeftInd) ? 1 : 0;
            right_raw = (!digifiz_reg_in.blinkRightInd) ? 1 : 0;
        }
        else
        {
            left_raw = 1;
            right_raw = 1;
        }
    }

    if (digifiz_parameters.signalOptions_enable_firmware_blinkers.value)
    {
        uint32_t now = millis();
        update_turn_blink_state(now);

        uint8_t left_active = is_turn_signal_active(&left_turn_latch, left_raw, now);
        uint8_t right_active = is_turn_signal_active(&right_turn_latch, right_raw, now);

        display.left_turn_ind = left_active ? turn_blink_state : 0;
        display.right_turn_ind = right_active ? turn_blink_state : 0;
    }
    else
    {
        display.left_turn_ind = filter_turn_indicator(&left_turn_indicator_filter, left_raw);
        display.right_turn_ind = filter_turn_indicator(&right_turn_indicator_filter, right_raw);
    }

    uint8_t brakes_raw = digifiz_reg_in.brakesInd ? 0 : 1;
    uint8_t foglight2_raw = digifiz_reg_in.lightsHeatInd ? 0 : 1;

    display.brakes_ind = filter_general_indicator(&brakes_indicator_filter, brakes_raw);
    display.foglight_ind2 = filter_general_indicator(&foglight2_indicator_filter, foglight2_raw);
}

static void getColorBySegmentNumber(ColoringScheme* c_ptr, uint16_t segment, uint8_t* r, uint8_t* g, uint8_t* b)
{
    uint16_t startSegment = 0;
    uint16_t endSegment = 0;
    for (uint16_t i=0;i!=49;i++)
    {
        endSegment = c_ptr->scheme[i].end_segment;
        if ((segment>=startSegment)&&(segment<endSegment))
        {
            if (c_ptr->scheme[i].basecolor_enabled==1)
            {
                (*r) = maincolor_r;
                (*g) = maincolor_g;
                (*b) = maincolor_b;
            }
            else if (c_ptr->scheme[i].basecolor_enabled==2)
            {
                (*r) = maincolor_r/3;
                (*g) = maincolor_g/3;
                (*b) = maincolor_b/3;
            }
            else if (c_ptr->scheme[i].basecolor_enabled==3)
            {
                (*r) = backcolor_r;
                (*g) = backcolor_g;
                (*b) = backcolor_b;
            }
            else
            {
                (*r) = c_ptr->scheme[i].r;
                (*g) = c_ptr->scheme[i].g;
                (*b) = c_ptr->scheme[i].b;
            }
        }
        startSegment = endSegment;
    }
}

uint16_t led_num = 0;

static uint8_t r_colors_default[DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS];
static uint8_t g_colors_default[DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS];
static uint8_t b_colors_default[DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS];

static uint8_t r_colors_active[DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS];
static uint8_t g_colors_active[DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS];
static uint8_t b_colors_active[DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS];

static void restore_speed_digit_colors(void)
{
    speed_alert_color_active = false;
}

static void apply_speed_alert_colors(uint8_t r, uint8_t g, uint8_t b)
{
    speed_alert_color_active = true;
}

static void refresh_speed_digit_colors(uint16_t speed_value)
{
    if (!digifiz_parameters.speedColorChangeEnable.value) {
        restore_speed_digit_colors();
        return;
    }

    uint16_t threshold = digifiz_parameters.speedColorChangeThreshold.value;
    bool should_activate = speed_value >= threshold;

    if (should_activate) {
        apply_speed_alert_colors(
            digifiz_parameters.speedAlertColor_r.value,
            digifiz_parameters.speedAlertColor_g.value,
            digifiz_parameters.speedAlertColor_b.value);
    } else {
        restore_speed_digit_colors();
    }
}

static void update_speed_digit_color_override(uint16_t speed_value)
{
    refresh_speed_digit_colors(speed_value);
}

static void apply_speed_digit_override_color(uint8_t r, uint8_t g, uint8_t b)
{
    
}

void compileColorScheme(void)
{
    led_num = 0;
    digifizStandard.scheme[redline_scheme_id-1].end_segment = COLORING_SCHEME_REDLINING_LIMIT-
                                                digifiz_parameters.rpmOptions_redline_segments.value;
    if ((digifiz_parameters.mainc_r.value==0)&&
        (digifiz_parameters.mainc_g.value==0)&&
        (digifiz_parameters.mainc_b.value==0))
    {
        digifiz_parameters.mainc_r.value = 180;
        digifiz_parameters.mainc_g.value = 240;
        digifiz_parameters.mainc_b.value = 6;
    }
    maincolor_r = digifiz_parameters.mainc_r.value;
    maincolor_g = digifiz_parameters.mainc_g.value;
    maincolor_b = digifiz_parameters.mainc_b.value;

    backcolor_r = digifiz_parameters.backc_r.value;
    backcolor_g = digifiz_parameters.backc_g.value;
    backcolor_b = digifiz_parameters.backc_b.value;

    // Check if custom scheme is enabled
    if (digifiz_parameters.useCustomScheme.value) {
        nvs_handle_t nvs_handle;
        esp_err_t err = nvs_open("digifiz", NVS_READWRITE, &nvs_handle);
        if (err == ESP_OK) {
            size_t required_size = 0;
            // First get the size of the stored JSON
            err = nvs_get_blob(nvs_handle, "color_scheme", NULL, &required_size);
            if (err == ESP_OK) {
                nvs_get_blob(nvs_handle, "color_scheme", (uint8_t*)&digifizCustom, &required_size);
                ESP_LOGI(LOG_TAG, "Custom colors scheme load ok.");
            }
            nvs_close(nvs_handle);
        }
    }

    // If custom scheme is not enabled or failed to load, use standard scheme
    uint8_t *ptr = (uint8_t*)&display;
    for (uint16_t i = 0; i != sizeof(DigifizNextDisplay); i++)
    {
        for (uint16_t j = 0; j != 8; j++)
        {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;
            if (digifiz_parameters.useCustomScheme.value)
                getColorBySegmentNumber(&digifizCustom, led_num,&r,&g,&b);
            else
                getColorBySegmentNumber(&digifizStandard, led_num,&r,&g,&b);
            r_colors_default[led_num] = r;
            g_colors_default[led_num] = g;
            b_colors_default[led_num] = b;

            r_colors_active[led_num] = r;
            g_colors_active[led_num] = g;
            b_colors_active[led_num] = b;
            led_num+=1;
            if (led_num>(DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS-1))
            {
                led_num=DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS-1;
                break;
            }
        }
    }
    speed_alert_color_active = false;
}

// Fire up the Digifiz system
void fireDigifiz() {
    effect_state.effect = digifiz_parameters.ledEffect_type.value;
    if (effect_state.effect != LED_EFFECT_NONE)
    {
        effect_state.hue = digifiz_parameters.ledEffect_hue.value;
        effect_state.saturation = digifiz_parameters.ledEffect_saturation.value;
        effect_state.value = digifiz_parameters.ledEffect_value.value;
        for (uint16_t i = 0; i < (DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS); i++)
        {
            led_rgb_t col = led_effect_compute(&effect_state, i);
            led_strip_set_pixel(led_strip, i,
                ((uint32_t)col.r * (uint32_t)backlightLevel) / 100,
                ((uint32_t)col.g * (uint32_t)backlightLevel) / 100,
                ((uint32_t)col.b * (uint32_t)backlightLevel) / 100);
        }
    }
    else
    {
        for (uint16_t i = 0; i < (DIGIFIZ_DISPLAY_NEXT_LEDS + DIGIFIZ_BACKLIGHT_LEDS); i++)
        {
            uint8_t r = r_colors_active[i];
            uint8_t g = g_colors_active[i];
            uint8_t b = b_colors_active[i];
            led_strip_set_pixel(led_strip, i,
                ((uint32_t)r * (uint32_t)backlightLevel) / 100,
                ((uint32_t)g * (uint32_t)backlightLevel) / 100,
                ((uint32_t)b * (uint32_t)backlightLevel) / 100);
        }
    }

    led_strip_refresh(led_strip);
    led_effect_step(&effect_state, 1.0f);
}

#endif
