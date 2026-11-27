#include "display_next.h"

#ifdef DIGIFIZ_NCA_DISPLAY

#include <math.h>

#include "driver/uart.h"
#include "esp_check.h"
#include "millis.h"
#include "params.h"
#include "reg_inout.h"
#include "vehicle_data.h"
#include <nca/nca.h>

#define TAG "display_nca"
#define NCA_UART_NUM UART_NUM_1
#define NCA_UART_TX_PIN 43
#define NCA_UART_RX_PIN UART_PIN_NO_CHANGE
#define NCA_UART_BAUD 250000
#define NCA_UART_RX_BUFFER_SIZE 256

#define NCA_FAST_PERIOD_MS 33u
#define NCA_SPEED_PERIOD_MS 200u
#define NCA_INDICATOR_PERIOD_MS 100u
#define NCA_SLOW_PERIOD_MS 1000u
#define NCA_GENERIC_PERIOD_MS 250u

ColoringScheme digifizCustom;
ColoringScheme digifizStandard;

typedef struct {
    bool oil;
    bool brakes;
    bool front_fog;
    bool rear_fog;
    bool rear_window_heat;
    bool high_beam;
    bool lights;
    bool left_turn;
    bool right_turn;
    bool battery;
    bool mfa1;
    bool mfa2;
    bool check_engine;
    bool low_fuel;
    bool backlight;
    int16_t mfa_value;
    uint16_t clock_hhmm;
    uint8_t brightness;
    int8_t gear;
    uint8_t mfa_page;
    uint32_t odometer_km;
} NcaDisplayState;

static NcaDisplayState state;
static bool uart_ready;
static uint16_t sequence[256];
static uint32_t last_rpm_ms;
static uint32_t last_speed_ms;
static uint32_t last_indicators_ms;
static uint32_t last_slow_ms;
static uint32_t last_generic_ms;
static uint8_t next_generic;

static int16_t deci_saturated(double value)
{
    if (!isfinite(value)) return 0;
    value *= 10.0;
    if (value > INT16_MAX) return INT16_MAX;
    if (value < INT16_MIN) return INT16_MIN;
    return (int16_t)lround(value);
}

static uint16_t u16_saturated(double value, double scale)
{
    if (!isfinite(value) || value <= 0.0) return 0;
    value *= scale;
    if (value > UINT16_MAX) return UINT16_MAX;
    return (uint16_t)lround(value);
}

static void send_payload(uint8_t topic, const uint8_t *payload, size_t payload_size)
{
    uint8_t frame[NCA_FRAME_OVERHEAD + NCA_PUBLISH_INDICATOR_PAYLOAD_SIZE];
    size_t frame_size = nca_frame_encode(NCA_BROADCAST_ADDRESS, NCA_CMD_PUBLISH,
                                         payload, (uint16_t)payload_size,
                                         frame, sizeof(frame));
    if (frame_size != 0u) {
        uart_write_bytes(NCA_UART_NUM, frame, frame_size);
        sequence[topic]++;
    }
}

static void publish_u16(uint8_t topic, uint16_t value)
{
    uint8_t payload[NCA_PUBLISH_SCALAR_PAYLOAD_SIZE];
    size_t size = nca_publish_encode_u16(payload, sizeof(payload), topic,
                                         sequence[topic], value);
    if (size != 0u) send_payload(topic, payload, size);
}

static void publish_deci(uint8_t topic, double value)
{
    uint8_t payload[NCA_PUBLISH_SCALAR_PAYLOAD_SIZE];
    size_t size = nca_publish_encode_deci(payload, sizeof(payload), topic,
                                          sequence[topic], deci_saturated(value));
    if (size != 0u) send_payload(topic, payload, size);
}

static void publish_odometer(void)
{
    uint8_t payload[NCA_PUBLISH_U32_PAYLOAD_SIZE];
    uint32_t deci_km = state.odometer_km > UINT32_MAX / 10u
        ? UINT32_MAX : state.odometer_km * 10u;
    size_t size = nca_publish_encode_u32(payload, sizeof(payload), NCA_TOPIC_ODOMETER,
                                         sequence[NCA_TOPIC_ODOMETER], deci_km);
    if (size != 0u) send_payload(NCA_TOPIC_ODOMETER, payload, size);
}

static void publish_indicators(void)
{
    uint32_t low = 0;
    uint32_t high = 0;
#define SET_INDICATOR(condition, bit) do { if (condition) low |= UINT32_C(1) << (bit); } while (0)
    SET_INDICATOR(state.lights, NCA_INDICATOR_LIGHTS_ON_BIT);
    SET_INDICATOR(state.front_fog, NCA_INDICATOR_FRONT_FOG_BIT);
    SET_INDICATOR(state.rear_fog, NCA_INDICATOR_REAR_FOG_BIT);
    SET_INDICATOR(state.rear_window_heat, NCA_INDICATOR_REAR_WINDOW_HEAT_BIT);
    SET_INDICATOR(state.high_beam, NCA_INDICATOR_HIGH_BEAM_BIT);
    SET_INDICATOR(state.left_turn, NCA_INDICATOR_LEFT_TURN_BIT);
    SET_INDICATOR(state.right_turn, NCA_INDICATOR_RIGHT_TURN_BIT);
    SET_INDICATOR(state.brakes, NCA_INDICATOR_BRAKE_BIT);
    SET_INDICATOR(state.oil, NCA_INDICATOR_OIL_PRESSURE_BIT);
    SET_INDICATOR(state.battery, NCA_INDICATOR_BATTERY_BIT);
    SET_INDICATOR(state.mfa1, NCA_INDICATOR_MFA1_BIT);
    SET_INDICATOR(state.mfa2, NCA_INDICATOR_MFA2_BIT);
    SET_INDICATOR(state.check_engine, NCA_INDICATOR_CHECK_ENGINE_BIT);
    SET_INDICATOR(state.low_fuel, NCA_INDICATOR_LOW_FUEL_BIT);
    SET_INDICATOR(state.low_fuel, NCA_INDICATOR_FUEL_LOW_BIT);
    SET_INDICATOR(state.backlight, NCA_INDICATOR_BACKLIGHT_BIT);
    SET_INDICATOR(get_oil_temp_c() > 130.0, NCA_INDICATOR_OIL_TEMPERATURE_HIGH_BIT);
    SET_INDICATOR(get_ambient_temp_c() > 50.0, NCA_INDICATOR_OUTDOOR_TEMPERATURE_HIGH_BIT);
    SET_INDICATOR(get_coolant_temp_c() > digifiz_parameters.coolantMax.value,
                  NCA_INDICATOR_COOLANT_TEMPERATURE_HIGH_BIT);
#undef SET_INDICATOR
    uint8_t payload[NCA_PUBLISH_INDICATOR_PAYLOAD_SIZE];
    size_t size = nca_publish_encode_indicators(payload, sizeof(payload),
                                                sequence[NCA_TOPIC_INDICATORS], low, high);
    if (size != 0u) send_payload(NCA_TOPIC_INDICATORS, payload, size);
}

static void send_mode_interactive(void)
{
    const uint8_t mode = NCA_MODE_INTERACTIVE;
    uint8_t frame[NCA_FRAME_OVERHEAD + 1u];
    size_t size = nca_frame_encode(NCA_BROADCAST_ADDRESS, NCA_CMD_SET_MODE,
                                   &mode, 1u, frame, sizeof(frame));
    if (size != 0u) uart_write_bytes(NCA_UART_NUM, frame, size);
}

void nca_uart_publisher_init(void)
{
    if (uart_ready) return;
    const uart_config_t config = {
        .baud_rate = NCA_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_param_config(NCA_UART_NUM, &config));
    ESP_ERROR_CHECK(uart_set_pin(NCA_UART_NUM, NCA_UART_TX_PIN, NCA_UART_RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(NCA_UART_NUM, NCA_UART_RX_BUFFER_SIZE, 0, 0, NULL, 0));
    uart_ready = true;
    send_mode_interactive();
    ESP_LOGI(TAG, "NCA publisher initialized on UART%d TX GPIO%d at %d baud",
             NCA_UART_NUM, NCA_UART_TX_PIN, NCA_UART_BAUD);
}

void nca_uart_publisher_trigger(void)
{
    if (!uart_ready) return;
    uint32_t now = millis();
    if ((uint32_t)(now - last_rpm_ms) >= NCA_FAST_PERIOD_MS) {
        last_rpm_ms = now;
        publish_u16(NCA_TOPIC_RPM, u16_saturated(get_rpm(), 1.0));
    }
    if ((uint32_t)(now - last_speed_ms) >= NCA_SPEED_PERIOD_MS) {
        last_speed_ms = now;
        publish_u16(NCA_TOPIC_SPEED, u16_saturated(get_speed_kmh(), 10.0));
    }
    if ((uint32_t)(now - last_indicators_ms) >= NCA_INDICATOR_PERIOD_MS) {
        last_indicators_ms = now;
        publish_indicators();
    }
    if ((uint32_t)(now - last_slow_ms) >= NCA_SLOW_PERIOD_MS) {
        last_slow_ms = now;
        publish_deci(NCA_TOPIC_OIL_TEMPERATURE, get_oil_temp_c());
        publish_deci(NCA_TOPIC_OUTDOOR_TEMPERATURE, get_ambient_temp_c());
        publish_deci(NCA_TOPIC_COOLANT_TEMPERATURE, get_coolant_temp_c());
        publish_deci(NCA_TOPIC_FUEL_LEVEL, get_fuel_level_l());
        publish_odometer();
    }
    if ((uint32_t)(now - last_generic_ms) >= NCA_GENERIC_PERIOD_MS) {
        last_generic_ms = now;
        switch (next_generic) {
        case 0: publish_deci(NCA_TOPIC_MFA_VALUE, state.mfa_value); break;
        case 1: publish_deci(NCA_TOPIC_MFA_CLOCK_VALUE, state.clock_hhmm); break;
        case 2: publish_deci(NCA_TOPIC_FUEL_CONSUMPTION, get_fuel_consumption_l100()); break;
        case 3: publish_deci(NCA_TOPIC_INTAKE_VOLTAGE, get_intake_voltage_v()); break;
        case 4: publish_deci(NCA_TOPIC_FUEL_PRESSURE, get_fuel_pressure_bar()); break;
        case 5: publish_deci(NCA_TOPIC_UPTIME_HOURS, get_uptime_h()); break;
        case 6: publish_deci(NCA_TOPIC_SPEED_RAW, get_speed_raw()); break;
        case 7: publish_deci(NCA_TOPIC_BRIGHTNESS_PERCENT, state.brightness); break;
        case 8: publish_deci(NCA_TOPIC_GEAR, state.gear); break;
        default: publish_deci(NCA_TOPIC_MFA_PAGE, state.mfa_page); break;
        }
        next_generic = (uint8_t)((next_generic + 1u) % NCA_TOPIC_GENERIC_COUNT);
    }
}

void initDisplay(void)
{
    memset(&state, 0, sizeof(state));
    /* GPIO21 was the WS2812 output. Leave it high impedance in NCA builds. */
    gpio_reset_pin(WSDATA_GPIO_PIN);
    gpio_set_direction(WSDATA_GPIO_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(WSDATA_GPIO_PIN, GPIO_FLOATING);
    nca_uart_publisher_init();
}

void deinit_leds(void) { gpio_reset_pin(WSDATA_GPIO_PIN); }
void compileColorScheme(void) {}
void fireDigifiz(void) {}
void setRPM(uint32_t value) { (void)value; }
void setRPMData(uint16_t value) { (void)value; }
void setSpeedometerData(uint16_t value) { (void)value; }
void setSpeedometerGear(int8_t gear) { state.gear = gear; }
void setCoolantData(uint16_t value) { (void)value; }
void setFuel(uint8_t litres) { (void)litres; }
void setClockData(uint8_t hours, uint8_t minutes) { state.clock_hhmm = (uint16_t)(hours * 100u + minutes); }
void setMFAClockData(uint8_t hours, uint8_t minutes) { (void)hours; (void)minutes; }
void setMFADisplayedNumber(int16_t value) { state.mfa_value = value; }
void setDot(bool value) { (void)value; }
void setFloatDot(bool value) { (void)value; }
void setMileage(uint32_t mileage) { state.odometer_km = mileage; }
void setMFAType(uint8_t type) { state.mfa_page = type; }
void displayMFAType(uint8_t type) { state.mfa_page = type; }
void displayMFAClock(void) {}
void setMFABlock(uint8_t block)
{
    state.mfa1 = (block & 1u) != 0u;
    state.mfa2 = !state.mfa1;
}
void setBrightness(uint8_t level) { state.brightness = level; }
void resetBrightness(void) {}
void setRefuelSign(bool on) { state.low_fuel = on; }
void setRefuelGear(int8_t gear) { state.gear = gear; }
void setCheckEngine(bool on) { state.check_engine = on; }
void applyCheckEngineAction(void) {}
void setBacklight(bool on) { state.backlight = on; }
void setServiceDisplayData(uint8_t data) { (void)data; }
void setOilIndicator(bool on) { state.oil = on; }
void setBrakesIndicator(bool on) { state.brakes = on; }
void setHeatLightsIndicator(bool on) { state.front_fog = on; }
void setBackLightsHeatIndicator(bool on) { state.rear_fog = on; }
void setBackWindowHeatIndicator(bool on) { state.rear_window_heat = on; }

void processIndicators(void)
{
    state.brakes = !digifiz_reg_in.brakesInd;
    state.front_fog = !digifiz_reg_in.fogLightsInd;
    state.rear_fog = !digifiz_reg_in.lightsHeatInd;
    state.rear_window_heat = !digifiz_reg_in.glheatInd;
    state.high_beam = !digifiz_reg_in.farLightsInd;
    state.lights = !digifiz_reg_in.lightsInd;
    state.battery = !digifiz_reg_in.batteryInd;
    if (digifiz_parameters.signalOptions_use_blink_alt_in.value) {
        state.left_turn = !digifiz_reg_in.fogLightsInd;
        state.right_turn = !state.left_turn && !digifiz_reg_in.glheatInd;
    } else if (digifiz_reg_in.blinkAll) {
        state.left_turn = !digifiz_reg_in.blinkLeftInd;
        state.right_turn = !digifiz_reg_in.blinkRightInd;
    } else {
        state.left_turn = true;
        state.right_turn = true;
    }
}

/* ReFIZ-only web diagnostic hooks remain link-safe but unavailable in NCA. */
void refiz_uart_sender_init(void) {}
void refiz_uart_sender_trigger(void) {}
void refiz_diagnostic_apply(bool enabled, const uint8_t *payload, bool reset)
{ (void)enabled; (void)payload; (void)reset; }
void refiz_diagnostic_get_state(bool *enabled, uint8_t *payload)
{ if (enabled) *enabled = false; (void)payload; }

#endif /* DIGIFIZ_NCA_DISPLAY */
