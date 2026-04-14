#ifndef DISPLAY_NEXT_H
#define DISPLAY_NEXT_H
/**
 * @file display_next.h
 * @brief Digifiz Next LED display rendering and indicator control API.
 */
#ifdef __cplusplus
extern "C" {
#endif


#include "setup.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "setup.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#ifdef DIGIFIZ_NEXT_DISPLAY

#include "adc.h"
#include "mfa.h"
#include "fuel_pressure.h"

/** @brief Enable verbose display debug logs when set to non-zero. */
#define DEBUG  1

/** @brief GPIO pin for MFA1 indicator input. */
#define MFA1_PIN 24
/** @brief GPIO pin for MFA2 indicator input. */
#define MFA2_PIN 25

/** @brief Analog pin used to read dashboard brightness level. */
#define BRIGHTNESS_IN_PIN A9 //PK1
/** @brief GPIO controlling the dashboard backlight power stage. */
#define BACKLIGHT_CTL_PIN 26 //PA4

/** @brief WS2812 data output GPIO for the Digifiz Next display chain. */
#define WSDATA_GPIO_PIN 21
/** @brief Number of LEDs used by the main display area. */
#define DIGIFIZ_DISPLAY_NEXT_LEDS 271
/** @brief Number of LEDs used by the auxiliary backlight strip. */
#define DIGIFIZ_BACKLIGHT_LEDS 32

/** @brief Compile-time flag to enable writing WS2812 display LEDs. */
#define USE_DISPLAY_LEDS

/**
 * @brief Bit masks for 7-segment glyphs used by Digifiz number rendering.
 */
enum 
{
    /** @brief Encoded glyph for digit '0'. */
    DIGIT_NUMBER_0 = 0b0111111,
    /** @brief Encoded glyph for digit '1'. */
    DIGIT_NUMBER_1 = 0b0011000,
    /** @brief Encoded glyph for digit '2'. */
    DIGIT_NUMBER_2 = 0b1110110,
    /** @brief Encoded glyph for digit '3'. */
    DIGIT_NUMBER_3 = 0b1111100,
    /** @brief Encoded glyph for digit '4'. */
    DIGIT_NUMBER_4 = 0b1011001,
    /** @brief Encoded glyph for digit '5'. */
    DIGIT_NUMBER_5 = 0b1101101,
    /** @brief Encoded glyph for digit '6'. */
    DIGIT_NUMBER_6 = 0b1101111,
    /** @brief Encoded glyph for digit '7'. */
    DIGIT_NUMBER_7 = 0b0111000,
    /** @brief Encoded glyph for digit '8'. */
    DIGIT_NUMBER_8 = 0b1111111,
    /** @brief Encoded glyph for digit '9'. */
    DIGIT_NUMBER_9 = 0b1111101,
    /** @brief Empty (all segments off) glyph. */
    DIGIT_NUMBER_EMPTY = 0,
    /** @brief Minus sign glyph. */
    DIGIT_NUMBER_MINUS = 0b1000000,
};

/**
 * @brief Color scheme selector indexes used in @ref ColoringScheme.
 */
enum
{
    /** @brief Invalid/uninitialized scheme index. */
    COLOR_SCHEME_INVALID,
    /** @brief Speedometer value color scheme. */
    COLOR_SCHEME_SPEEDOMETER,
    /** @brief Clock/time digits color scheme. */
    COLOR_SCHEME_TIME,
    /** @brief RPM numeric digits color scheme. */
    COLOR_SCHEME_RPM,
    /** @brief RPM redline scale color scheme. */
    COLOR_SCHEME_RPM_REDLINE,
    /** @brief Temperature value color scheme. */
    COLOR_SCHEME_TEMPERATURE,
    /** @brief Fuel value color scheme. */
    COLOR_SCHEME_FUEL,
    /** @brief Refuel warning color scheme. */
    COLOR_SCHEME_REFUEL,
    /** @brief Odometer/mileage color scheme. */
    COLOR_SCHEME_MILEAGE,
    /** @brief RPM backlight bar color scheme. */
    COLOR_SCHEME_BACKLIGHT_RPM,
    /** @brief Main backlight color scheme. */
    COLOR_SCHEME_BACKLIGHT,
    /** @brief Generic indicator icons color scheme. */
    COLOR_SCHEME_SIGNALS,
    /** @brief Lights-on indicator color scheme. */
    COLOR_SCHEME_SIGNAL_LIGHTS,
    /** @brief Front fog light indicator color scheme. */
    COLOR_SCHEME_SIGNAL_FOGLIGHTS1,
    /** @brief Rear fog light indicator color scheme. */
    COLOR_SCHEME_SIGNAL_FOGLIGHTS2,
    /** @brief Rear window heater indicator color scheme. */
    COLOR_SCHEME_SIGNAL_GLASSHEAT,
    /** @brief High-beam indicator color scheme. */
    COLOR_SCHEME_SIGNAL_FARLIGHT,
    /** @brief Left turn indicator color scheme. */
    COLOR_SCHEME_SIGNAL_LEFT_TURN_IND,
    /** @brief Right turn indicator color scheme. */
    COLOR_SCHEME_SIGNAL_RIGHT_TURN_IND,
    /** @brief Brake warning indicator color scheme. */
    COLOR_SCHEME_SIGNAL_BRAKES_IND,
    /** @brief Oil pressure warning indicator color scheme. */
    COLOR_SCHEME_SIGNAL_OIL_IND,
    /** @brief Battery warning indicator color scheme. */
    COLOR_SCHEME_SIGNAL_BATTERY_IND,
    /** @brief MFA1 indicator color scheme. */
    COLOR_SCHEME_SIGNAL_MFA1_IND,
    /** @brief MFA2 indicator color scheme. */
    COLOR_SCHEME_SIGNAL_MFA2_IND,
    /** @brief Combined MFA indicator icons color scheme. */
    COLOR_SCHEME_SIGNAL_MFA_INDICATORS,
    /** @brief MFA numeric/text area color scheme. */
    COLOR_SCHEME_MFA,
    /** @brief Upper panel backlight color scheme. */
    COLOR_SCHEME_UPPER_BACKLIGHT,
    /** @brief Total number of color-scheme indexes. */
    __MAX_COLOR_SCHEME
};

/** @brief Max number of keyframe segments in one configurable color scheme. */
#define COLORING_SCHEME_MAX_ELEMENTS 50
/** @brief RPM segment index where redline coloring begins. */
#define COLORING_SCHEME_REDLINING_LIMIT 69

/** @brief One RGB keyframe entry for a display segment range. */
typedef struct RGBColoringElement
{
    /** @brief Red channel intensity (0..255). */
    uint8_t r;
    /** @brief Green channel intensity (0..255). */
    uint8_t g;
    /** @brief Blue channel intensity (0..255). */
    uint8_t b;
    /** @brief Last LED segment index covered by this element. */
    int16_t end_segment;
    /** @brief Scheme type/index this element belongs to. */
    uint8_t type;
    /** @brief Set to non-zero to preserve base color behavior. */
    uint8_t basecolor_enabled;
} RGBColoringElement;

/** @brief Expanded per-LED color table generated from scheme keyframes. */
typedef struct CompiledColoringScheme
{
    RGBColoringElement scheme[DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS];
} CompiledColoringScheme;

/** @brief Compressed color table made of keyframe segments. */
typedef struct ColoringScheme
{
    RGBColoringElement scheme[COLORING_SCHEME_MAX_ELEMENTS];
} ColoringScheme;

/** @brief Packed LED/segment state payload for the Digifiz Next cluster. */
typedef struct __attribute__((packed)) DigifizNextDisplay
{
    uint8_t coolant_backlight : 1; //1
    uint16_t coolant_value : 14; //15
    uint8_t vdo_backlight : 1; //16
    uint8_t mfa2_ind : 1; //17 
    uint8_t mfa1_ind : 1; //18
    uint8_t battery_ind : 1; //19
    uint8_t oil_ind : 1; //20
    uint8_t brakes_ind : 1; //21
    uint8_t right_turn_ind : 1; //22
    uint8_t left_turn_ind : 1; //23
    uint8_t farlight_ind : 1; //24
    uint8_t glassheat_ind : 1; //25
    uint8_t foglight_ind1 : 1; //26
    uint8_t foglight_ind2 : 1; //27
    uint8_t lights_on_ind : 1; //28
    uint8_t rpm_padding : 4; //32
    uint8_t rpm[5]; //72
    uint8_t rpm_last :5; //77
    uint32_t rpm_backlight: 18; //95
    uint8_t mileage_digit_1 : 7; //102
    uint8_t mileage_digit_2 : 7; //109
    uint8_t mileage_digit_3 : 7; //116
    uint8_t mileage_digit_4 : 7; //123
    uint8_t mileage_digit_5 : 7; //130
    uint8_t mileage_digit_6 : 7; //137
    uint8_t km_backlight : 1; //138
    uint8_t clock_digit_1 : 7; //145
    uint8_t clock_digit_2 : 7; //152
    uint8_t clock_dot : 2; //154
    uint8_t clock_digit_3 : 7; //161
    uint8_t clock_digit_4 : 7; //168
    uint8_t mfa_indicators : 7; //175
    uint8_t mfa_digit_1 : 7; //182
    uint8_t mfa_digit_2 : 7; //189
    uint8_t mfa_digit_3 : 7; //196
    uint8_t mfa_digit_4 : 7; //203
    uint8_t mfa_dots : 3; //206
    uint8_t fuel_digit_1 : 7; //213
    uint8_t fuel_digit_2 : 7; //220
    uint8_t fuel_ind : 1; //221
    uint8_t fuel_low_ind : 6; //227
    uint8_t speed_digit_1 : 7; //234
    uint8_t speed_digit_2 : 7; //241
    uint8_t speed_digit_3 : 7; //248
    uint8_t speed_digit_s1 : 7; //255
    uint8_t speed_digit_s2 : 7; //262
    uint8_t speed_digit_s3 : 7; //269
    uint8_t km_ind : 1; //270
    uint32_t backlight_leds:32; //282
} DigifizNextDisplay;

/** @brief Initialize display hardware and internal rendering buffers. */
void initDisplay(); 
/** @brief Update raw RPM bargraph payload bits. */
void setRPM(uint32_t rpmdata);
/** @brief Set dashboard clock digits (HH:MM). */
void setClockData(uint8_t clock_hours,uint8_t clock_minutes);
/** @brief Set MFA clock digits (HH:MM). */
void setMFAClockData(uint8_t mfa_clock_hours,uint8_t mfa_clock_minutes);
/** @brief Set currently displayed MFA numeric value. */
void setMFADisplayedNumber(int16_t data);
/** @brief Set displayed fuel amount (liters). */
void setFuel(uint8_t litres);
/** @brief Set RPM value for formatted display rendering. */
void setRPMData(uint16_t data);
/** @brief Set speedometer value for formatted display rendering. */
void setSpeedometerData(uint16_t data);
/** @brief Set currently estimated gear to display. */
void setSpeedometerGear(int8_t gear);
/** @brief Set displayed coolant temperature value. */
void setCoolantData(uint16_t data);
/** @brief Enable/disable the main decimal dot. */
void setDot(bool value);
/** @brief Enable/disable floating secondary dot indicator. */
void setFloatDot(bool value);
/** @brief Set odometer mileage value to show. */
void setMileage(uint32_t mileage);
/** @brief Set MFA page/type selector value. */
void setMFAType(uint8_t mfaType);
/** @brief Render glyph/icon matching the provided MFA type. */
void displayMFAType(uint8_t mfaType);
/** @brief Render MFA clock page using current MFA clock fields. */
void displayMFAClock();
/** @brief Select active MFA block identifier. */
void setMFABlock(uint8_t block);
/** @brief Apply brightness level to display and backlight LEDs. */
void setBrightness(uint8_t levels);
/** @brief Restore brightness to automatically calculated value. */
void resetBrightness();
/** @brief Toggle refuel indicator sign visibility. */
void setRefuelSign(bool onoff);
/** @brief Display gear hint during refuel/info mode. */
void setRefuelGear(int8_t gear);
/** @brief Set check-engine indicator target state. */
void setCheckEngine(bool onoff);
/** @brief Flush pending check-engine display action to outputs. */
void applyCheckEngineAction(void);
/** @brief Turn dashboard backlight LEDs on or off. */
void setBacklight(bool onoff);
/** @brief Set service/info display data byte. */
void setServiceDisplayData(uint8_t data);

/** @brief Build per-LED color lookup tables from configured schemes. */
void compileColorScheme(void);

/** @brief Push current framebuffer state to physical Digifiz display LEDs. */
void fireDigifiz();

/** @brief Set oil warning indicator state. */
void setOilIndicator(bool onoff);
/** @brief Set brakes warning indicator state. */
void setBrakesIndicator(bool onoff);
/** @brief Set heated-lights indicator state. */
void setHeatLightsIndicator(bool onoff);
/** @brief Set rear-lights heater indicator state. */
void setBackLightsHeatIndicator(bool onoff);
/** @brief Set rear-window heater indicator state. */
void setBackWindowHeatIndicator(bool onoff);
/** @brief Read indicator inputs and refresh indicator outputs. */
void processIndicators();

/** @brief Deinitialize WS2812/LED driver resources. */
void deinit_leds(void);

/** @brief User-customizable color scheme loaded from settings. */
extern ColoringScheme digifizCustom;
/** @brief Built-in default color scheme. */
extern ColoringScheme digifizStandard;

#ifdef __cplusplus
}
#endif
#endif
#endif
