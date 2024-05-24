#include "display_next.h"
#include "reg_inout.h"
#include "esp_log.h"

uint8_t selectedBrightness = 20;
uint32_t mRPMData = 4000;
uint8_t backlightOff = 0;

uint8_t backlightLevel = 30;

DigifizNextDisplay display;
static led_strip_handle_t led_strip;
float brightnessFiltered = 6.0f;

static void configure_led(void)
{
    ESP_LOGI(LOG_TAG, "Digifiz WS2812 LED init.");
     led_strip_config_t strip_config = {
        .strip_gpio_num = WSDATA_GPIO_PIN,
        .max_leds = DIGIFIZ_DISPLAY_NEXT_LEDS, // at least one LED on board
    };
    #if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
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
    
    //display.rpm_padding = 0xF;
    //display.rpm[0] = 0xFF;
    setFuel(99);
    configure_led();
    setMileage(123456);
    setRPMData(5);
    ESP_LOGI(LOG_TAG, "Digifiz WS2812 LED init OK.");
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
        display.clock_digit_2 = DIGIT_NUMBER_0;
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
        display.mfa_digit_2 = DIGIT_NUMBER_0;
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
    }
    display.mfa_digit_3 = number_mfa[(-data / 10) % 10];
    display.mfa_digit_4 = number_mfa[(-data / 1) % 10];
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
void setRPMData(uint16_t data) {
    // Implementation placeholder
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
            if (digifiz_parameters.digifiz_options&OPTION_FAHRENHEIT)
            {
              setMFADisplayedNumber((int16_t)getOilTemperatureFahrenheit());
              setFloatDot(false);
            }
            else if (digifiz_parameters.digifiz_options&OPTION_KELVIN)
            {
              setMFADisplayedNumber((int16_t)(getOilTemperature()+273.15f));
              setFloatDot(false);
            }
            else
            {
              setMFADisplayedNumber((int16_t)(getOilTemperature()));
              setFloatDot(true);
            }
            break;
        case MFA_STATE_AIR_TEMP:
            display.mfa_dots&=0b00;
            if (digifiz_parameters.digifiz_options&OPTION_FAHRENHEIT)
            {
              setMFADisplayedNumber((int16_t)getAmbientTemperatureFahrenheit());
              setFloatDot(false);
            }
            else if (digifiz_parameters.digifiz_options&OPTION_KELVIN)
            { 
              setMFADisplayedNumber((int16_t)(getAmbientTemperature()+273.15f));
              setFloatDot(false);
            }
            else
            {
              setMFADisplayedNumber((int16_t)getAmbientTemperature());
              setFloatDot(true);
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
    
}

uint16_t led_num = 0;
// Fire up the Digifiz system
void fireDigifiz() {
    led_num = 0;
    uint8_t *ptr = (uint8_t*)&display;
    for (uint16_t i = 0; i != sizeof(DigifizNextDisplay); i++)
    {
        for (uint16_t j = 0; j != 8; j++)
        {
            uint8_t bit = (ptr[i] >> j) & 1;
            if (bit)
                led_strip_set_pixel(led_strip, led_num, (60*((uint32_t)backlightLevel))/100,(80*((uint32_t)backlightLevel))/100,(2*((uint32_t)backlightLevel))/100);
            else
                led_strip_set_pixel(led_strip, led_num, 0,0,0);
            led_num+=1;
            if (led_num>DIGIFIZ_DISPLAY_NEXT_LEDS-1)
            {
                led_num=DIGIFIZ_DISPLAY_NEXT_LEDS-1;
                break;
            }
        }
    }
    led_strip_refresh(led_strip);
}
