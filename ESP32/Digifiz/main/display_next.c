#include "display_next.h"
#include "reg_inout.h"
#include "esp_log.h"
#include "mjs.h"
//If updating, do not display anything
#include "digifiz_ws_server.h"

#define TAG "display_next"
#define SCRIPT_KEY "display_next_script"

uint8_t selectedBrightness = 20;
uint32_t mRPMData = 4000;
uint8_t backlightOff = 0;

uint8_t backlightLevel = 30;

DigifizNextDisplay display;
static led_strip_handle_t led_strip;
float brightnessFiltered = 6.0f;

static uint8_t maincolor_r;
static uint8_t maincolor_g;
static uint8_t maincolor_b;
static uint8_t backcolor_r;
static uint8_t backcolor_g;
static uint8_t backcolor_b;
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

static CompiledColoringScheme digifizCustom = {};

// Function to run the mJS script
void run_mjs_script(const char *script) {
    struct mjs *mjs = mjs_create();

    mjs_val_t func = 0;
    mjs_val_t res = 0;

    //mjs_own(mjs, &func);

    // Load the script into mJS
    enum mjs_err err = mjs_exec(mjs, script, NULL);
    if (err != MJS_OK) {
        ESP_LOGE(TAG, "mJS execution error: %s", mjs_strerror(mjs, err));
        mjs_destroy(mjs);
        return;
    }

    // Call the setColors() function
    func = mjs_get(mjs, mjs_get_global(mjs), "setColors", 0);
    if (!mjs_is_function(func)) {
        ESP_LOGE(TAG, "Function setColors not found");
        mjs_destroy(mjs);
        return;
    }

    mjs_val_t result = mjs_call(mjs, &res, func, 0, 0);
    if (!mjs_is_array(result)) {
        ESP_LOGE(TAG, "setColors() did not return an array");
        mjs_destroy(mjs);
        return;
    }

    // Parse the result into the RGB array
    for (int i = 0; i < (DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS); i++) {
        mjs_val_t item = mjs_array_get(mjs, result, i);
        uint8_t r = mjs_get_int(mjs, mjs_get(mjs, item, "r", 0));
        uint8_t g = mjs_get_int(mjs, mjs_get(mjs, item, "g", 0));
        uint8_t b = mjs_get_int(mjs, mjs_get(mjs, item, "b", 0));
        digifizCustom.scheme[i].r = r;
        digifizCustom.scheme[i].g = g;
        digifizCustom.scheme[i].b = b;
    }

    ESP_LOGI(TAG, "RGB array initialized from mJS script");
    mjs_destroy(mjs);
}    

static ColoringScheme digifizStandard = {
    .scheme = {
        { 
            .r = 20,
            .g = 20,
            .b = 2,
            .end_segment = 1,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 2
        },
        {
            .r = 60,
            .g = 8,
            .b = 2,
            .end_segment = 15,
            .type = COLOR_SCHEME_TEMPERATURE,
            .basecolor_enabled = 1
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 16,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 1
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 17,
            .type = COLOR_SCHEME_SIGNAL_MFA1_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 18,
            .type = COLOR_SCHEME_SIGNAL_MFA2_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 240,
            .g = 2,
            .b = 2,
            .end_segment = 19,
            .type = COLOR_SCHEME_SIGNAL_BATTERY_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 240,
            .g = 2,
            .b = 2,
            .end_segment = 20,
            .type = COLOR_SCHEME_SIGNAL_OIL_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 240,
            .g = 2,
            .b = 2,
            .end_segment = 21,
            .type = COLOR_SCHEME_SIGNAL_BRAKES_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 22,
            .type = COLOR_SCHEME_SIGNAL_RIGHT_TURN_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 23,
            .type = COLOR_SCHEME_SIGNAL_LEFT_TURN_IND,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 2,
            .b = 120,
            .end_segment = 24,
            .type = COLOR_SCHEME_SIGNAL_FARLIGHT,
            .basecolor_enabled = 0
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 25,
            .type = COLOR_SCHEME_SIGNAL_GLASSHEAT,
            .basecolor_enabled = 0
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 26,
            .type = COLOR_SCHEME_SIGNAL_FOGLIGHTS2,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 240,
            .b = 2,
            .end_segment = 27,
            .type = COLOR_SCHEME_SIGNAL_FOGLIGHTS1,
            .basecolor_enabled = 0
        },
        { 
            .r = 2,
            .g = 80,
            .b = 2,
            .end_segment = 28,
            .type = COLOR_SCHEME_SIGNAL_LIGHTS,
            .basecolor_enabled = 0
        },
        { 
            .r = 60,
            .g = 8,
            .b = 2,
            .end_segment = COLORING_SCHEME_REDLINING_LIMIT,
            .type = COLOR_SCHEME_RPM,
            .basecolor_enabled = 1
        },
        { 
            .r = 120,
            .g = 8,
            .b = 2,
            .end_segment = 77,
            .type = COLOR_SCHEME_RPM_REDLINE,
            .basecolor_enabled = 0
        },
        { 
            .r = 20,
            .g = 20,
            .b = 2,
            .end_segment = 95,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 2
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 137,
            .type = COLOR_SCHEME_MILEAGE,
            .basecolor_enabled = 1
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 138,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 1
        },
        { 
            .r = 6,
            .g = 8,
            .b = 60,
            .end_segment = 168,
            .type = COLOR_SCHEME_TIME,
            .basecolor_enabled = 1
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 175,
            .type = COLOR_SCHEME_SIGNAL_MFA_INDICATORS,
            .basecolor_enabled = 1
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 206,
            .type = COLOR_SCHEME_MFA,
            .basecolor_enabled = 1
        },
        { 
            .r = 2,
            .g = 2,
            .b = 80,
            .end_segment = 221,
            .type = COLOR_SCHEME_FUEL,
            .basecolor_enabled = 1
        },
        { 
            .r = 80,
            .g = 2,
            .b = 2,
            .end_segment = 227,
            .type = COLOR_SCHEME_REFUEL,
            .basecolor_enabled = 0
        },
        { 
            .r = 60,
            .g = 8,
            .b = 2,
            .end_segment = 269,
            .type = COLOR_SCHEME_SPEEDOMETER,
            .basecolor_enabled = 1
        },
        { 
            .r = 20,
            .g = 20,
            .b = 2,
            .end_segment = 270,
            .type = COLOR_SCHEME_BACKLIGHT,
            .basecolor_enabled = 2
        },
        { 
            .r = 60,
            .g = 80,
            .b = 2,
            .end_segment = 282,
            .type = COLOR_SCHEME_UPPER_BACKLIGHT,
            .basecolor_enabled = 3
        },
    }
};

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
    display.backlight_leds = 0b111111111111;

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
    run_mjs_script(default_mjs_script);
    ESP_LOGI(LOG_TAG, "initDisplay ended");
}

// Set the RPM data
void setRPM(uint32_t rpmdata) {
    mRPMData=rpmdata;
}

// Set the clock data
void setClockData(uint8_t clock_hours, uint8_t clock_minutes) {
    // Implementation placeholder

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
    if (((clock_hours / 10) % 10)!=0)
    {
        display.clock_digit_1 = number_clock[(clock_hours / 10) % 10];
        display.clock_digit_2 = number_clock[(clock_hours / 1) % 10];
        display.clock_digit_3 = number_clock[(clock_minutes / 10) % 10];
        display.clock_digit_4 = number_clock[(clock_minutes / 1) % 10];
    }
    else
    {
        display.clock_digit_1 = DIGIT_NUMBER_0;
        display.clock_digit_2 = number_clock[(clock_hours / 1) % 10];
        display.clock_digit_3 = number_clock[(clock_minutes / 10) % 10];
        display.clock_digit_4 = number_clock[(clock_minutes / 1) % 10];
    }
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

// Set the RPM data
void setRPMData(uint16_t inp_d) {
    // Implementation placeholder
    if (inp_d>digifiz_parameters.maxRPM)
        inp_d = digifiz_parameters.maxRPM;
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
            setMFADisplayedNumber((uint16_t)(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]/3600));
            setFloatDot(false);
            break;
        case MFA_STATE_TRIP_L100KM:
            display.mfa_dots&=0b00;
            setMFADisplayedNumber((uint16_t)(digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock]*100));
            //setMFADisplayedNumber((uint16_t)(getCurrentIntakeFuelConsumption()*100.0f));
            setFloatDot(true);
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            display.mfa_dots&=0b00;
            setMFADisplayedNumber((uint16_t)fabs(digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock]));
            setFloatDot(false);
            break;
        case MFA_STATE_OIL_TEMP:
            display.mfa_dots&=0b00;
            if (digifiz_parameters.digifiz_options.option_fahrenheit)
            {
              setMFADisplayedNumber((int16_t)getOilTemperatureFahrenheit());
              setFloatDot(false);
            }
            else if (digifiz_parameters.digifiz_options.option_kelvin)
            {
              setMFADisplayedNumber((int16_t)(getOilTemperature()+273.15f));
              setFloatDot(false);
            }
            else
            {
              setMFADisplayedNumber((int16_t)(getOilTemperature()));
              setFloatDot(false);
            }
            break;
        case MFA_STATE_AIR_TEMP:
            display.mfa_dots&=0b00;
            if (digifiz_parameters.digifiz_options.option_fahrenheit)
            {
              setMFADisplayedNumber((int16_t)getAmbientTemperatureFahrenheit());
              setFloatDot(false);
            }
            else if (digifiz_parameters.digifiz_options.option_kelvin)
            { 
              setMFADisplayedNumber((int16_t)(getAmbientTemperature()+273.15f));
              setFloatDot(false);
            }
            else
            {
              setMFADisplayedNumber((int16_t)getAmbientTemperature());
              setFloatDot(false);
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

// Set the brightness level
void setBrightness(uint8_t levels) {
    //printf("Br: %u\n", levels);
    brightnessFiltered += 0.1f*((float)levels-brightnessFiltered);
    backlightLevel = (uint8_t)brightnessFiltered;
}

void resetBrightness()
{
    brightnessFiltered = 0.0f;
    backlightLevel = 0;
}

// Set the refuel sign status
void setRefuelSign(bool onoff) {
    display.fuel_low_ind = onoff ? 0b111111 : 0;
}

// Set the check engine sign status
void setCheckEngine(bool onoff) {
    // Implementation placeholder
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
    display.brakes_ind = onoff ? 1 : 0;
}
void setHeatLightsIndicator(bool onoff)
{
    display.foglight_ind1 = onoff ? 1 : 0;
}
void setBackLightsHeatIndicator(bool onoff)
{
    display.foglight_ind2 = onoff ? 1 : 0;
}
void setBackWindowHeatIndicator(bool onoff)
{
    display.glassheat_ind = onoff ? 1 : 0;
}
void processIndicators()
{
    if (digifiz_parameters.sign_options.use_blink_other_inputs)
    {
        if (!digifiz_reg_in.fogLightsInd)
        {
            display.left_turn_ind = 1;
        }
        else if (!digifiz_reg_in.glheatInd)
        {
            display.right_turn_ind = 1;
        }
        else
        {
            display.left_turn_ind = 0;
            display.right_turn_ind = 0;
        }
    }
    else
    {
        //Normal way:
        if (digifiz_reg_in.blinkAll)
        {
            display.left_turn_ind = 0;
            display.right_turn_ind = 0;
            if (!digifiz_reg_in.blinkLeftInd)
            {
                display.left_turn_ind = 1;
            }
            if (!digifiz_reg_in.blinkRightInd)
            {
                display.right_turn_ind = 1;
            }
        }
        else
        {
            display.left_turn_ind = 1;
            display.right_turn_ind = 1;
        }
    }
    display.brakes_ind = digifiz_reg_in.brakesInd ? 0 : 1;
    display.foglight_ind2 = digifiz_reg_in.lightsHeatInd ? 0 : 1;
}

void getColorBySegmentNumber(uint16_t segment, uint8_t* r, uint8_t* g, uint8_t* b)
{
    uint16_t startSegment = 0;
    uint16_t endSegment = 0;
    for (uint16_t i=0;i!=49;i++)
    {
        endSegment = digifizStandard.scheme[i].end_segment;
        if ((segment>=startSegment)&&(segment<endSegment))
        {
            if (digifizStandard.scheme[i].basecolor_enabled==1)
            {
                (*r) = maincolor_r;
                (*g) = maincolor_g;
                (*b) = maincolor_b;
            }
            else if (digifizStandard.scheme[i].basecolor_enabled==2)
            {
                (*r) = maincolor_r/3;
                (*g) = maincolor_g/3;
                (*b) = maincolor_b/3;
            }
            else if (digifizStandard.scheme[i].basecolor_enabled==3)
            {
                (*r) = backcolor_r;
                (*g) = backcolor_g;
                (*b) = backcolor_b;
            }
            else
            {
                (*r) = digifizStandard.scheme[i].r;
                (*g) = digifizStandard.scheme[i].g;
                (*b) = digifizStandard.scheme[i].b;
            }
        }
        startSegment = endSegment;
    }
}

uint16_t led_num = 0;
// Fire up the Digifiz system
void fireDigifiz() {
    led_num = 0;
    digifizStandard.scheme[redline_scheme_id-1].end_segment = COLORING_SCHEME_REDLINING_LIMIT-
                                                digifiz_parameters.rpm_options.redline_segments;
    if ((digifiz_parameters.mainc_r==0)&&
        (digifiz_parameters.mainc_g==0)&&
        (digifiz_parameters.mainc_b==0))
    {
        digifiz_parameters.mainc_r = 180;
        digifiz_parameters.mainc_g = 240;
        digifiz_parameters.mainc_b = 6;
    }
    maincolor_r = digifiz_parameters.mainc_r;
    maincolor_g = digifiz_parameters.mainc_g;
    maincolor_b = digifiz_parameters.mainc_b;

    backcolor_r = digifiz_parameters.backc_r;
    backcolor_g = digifiz_parameters.backc_g;
    backcolor_b = digifiz_parameters.backc_b;

    uint8_t *ptr = (uint8_t*)&display;
    for (uint16_t i = 0; i != sizeof(DigifizNextDisplay); i++)
    {
        for (uint16_t j = 0; j != 8; j++)
        {
            uint8_t bit = (ptr[i] >> j) & 1;
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            getColorBySegmentNumber(led_num,&r,&g,&b);
            if (get_update_in_progress())
                bit=0;
            //led_strip_set_pixel(led_strip, led_num, 10,10,10);
            if (bit)
            {
                led_strip_set_pixel(led_strip, led_num, ((uint32_t)r*((uint32_t)backlightLevel))/100,
                    ((uint32_t)g*((uint32_t)backlightLevel))/100,
                    ((uint32_t)b*((uint32_t)backlightLevel))/100);
            }
            else
            {
                led_strip_set_pixel(led_strip, led_num, 0,0,0);
            }
            led_num+=1;
            if (led_num>(DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS-1))
            {
                led_num=DIGIFIZ_DISPLAY_NEXT_LEDS+DIGIFIZ_BACKLIGHT_LEDS-1;
                break;
            }
        }
    }
    led_strip_refresh(led_strip);
}
