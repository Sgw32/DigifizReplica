#include "mfa.h"
#include "esp_log.h"
#include "millis.h"
#include "reg_inout.h"
#include "driver/gpio.h"
#include <time.h>

uint8_t uptimeDisplayEnabled = 0;
uint8_t mfaMemorySelected = 0;

uint8_t bMFAMode = 0;
uint8_t bMFABlock = 0;
uint8_t bMFAReset = 0;
uint8_t bMFASensor = 0;

uint8_t prevMFAMode = 0;
uint8_t prevMFABlock = 0;
uint8_t prevMFAReset = 0;
uint8_t prevMFASensor = 0;

uint8_t sensorPressed = 0;
uint32_t pressSensorTime = 0;

extern int16_t seconds_block1;
extern int16_t seconds_block2;
extern struct tm saved_time1;
extern struct tm saved_time2;
// Initialize the MFA (Multi-Function Display)
void initMFA() {
    // Implementation placeholder
    ESP_LOGI(LOG_TAG, "initMFA started");
    gpio_config_t io_conf = {};
    // Configure the IOMUX register for pad GPIO_INPUT_PIN (pad 38) as a GPIO pin
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = TOUCH_PIN;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);
    // Set the GPIO as a input
    gpio_set_direction(TOUCH_PIN, GPIO_MODE_INPUT);
    ESP_LOGI(LOG_TAG, "initMFA ended");
}

// Process MFA data
void processMFA() 
{
    bMFAMode = digifiz_reg_in.mfaMode;
    bMFABlock = digifiz_reg_in.mfaBlock;
    bMFAReset = digifiz_reg_in.mfaReset;
    bMFASensor = 0;//gpio_get_level(TOUCH_PIN);
#ifndef DISABLE_MANUFACTURER_MFA
    if (digifiz_parameters.digifiz_options.mfa_manufacturer)
    {
      if ((bMFAMode==0)&&(prevMFAMode==1))
      {
          //Pressed MFA Mode
          ESP_LOGI(LOG_TAG,  "Mode=1");
          pressMFAMode();
      } 
      digifiz_parameters.mfaBlock = (bMFABlock==1) ? 1 : 0;
      
      if ((bMFAReset==0)&&(prevMFAReset==1))
      {
          //Pressed MFA Reset
          ESP_LOGI(LOG_TAG,  "Reset=1");
          pressMFAReset();
      }
    }
#endif
#ifndef DISABLE_SENSOR_BUTTON
    if ((bMFASensor==1)&&(prevMFASensor==0))
    {
        //Pressed MFA Sensor(on Digifiz)
        sensorPressed = 1;
        //TODO
        pressSensorTime = millis();
    }
    
    if ((bMFASensor==0) //logic is inversed!
        &&sensorPressed)
    {
        sensorPressed = 0;
        if ((millis() - pressSensorTime)<1000)
            pressMFASensorShort();
        else if ((millis() - pressSensorTime)<3000)
            pressMFASensorLong();
        else if ((millis() - pressSensorTime)<7000)
            pressMFASensorSuperLong();
        else if ((millis() - pressSensorTime)>7000)
            pressMFASensorSuperSuperLong();
    }
#endif
    prevMFAMode = bMFAMode;
    prevMFABlock = bMFABlock;
    prevMFAReset = bMFAReset;
    prevMFASensor = bMFASensor;
}

// Simulate pressing MFA mode button
void pressMFAMode() {
    digifiz_parameters.mfaState+=1;  
#ifdef AUDI_RED_DISPLAY
    if (digifiz_parameters.mfaState>6) // 0 1 2 3 4 5 6
        digifiz_parameters.mfaState = 0;
#else
    if (digifiz_parameters.mfaState>5) // 0 1 2 3 4 5
        digifiz_parameters.mfaState = 0;
#endif
    saveParameters();
}

// Simulate pressing MFA block button
void pressMFABlock() {
    digifiz_parameters.mfaBlock+=1;
    digifiz_parameters.mfaBlock&=0x1;
}

// Simulate pressing MFA reset button
void pressMFAReset() {
    time_t current_time_t;
    time(&current_time_t);
    switch(digifiz_parameters.mfaState)
    {
        case MFA_STATE_TRIP_DURATION:
            digifiz_parameters.duration[digifiz_parameters.mfaBlock] = 0;
            if (digifiz_parameters.mfaBlock==0)
            {
                seconds_block1 = 0;
                // Convert current time to struct tm
                localtime_r(&current_time_t, &saved_time1);
            }
            else
            {
                seconds_block2 = 0;
                // Convert current time to struct tm
                localtime_r(&current_time_t, &saved_time2);
            }
            break;
        case MFA_STATE_TRIP_DISTANCE:
            digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock] = 0;
            break;
        case MFA_STATE_TRIP_L100KM:
            digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock] = 0;
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock] = 0;
            break;
#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
        case MFA_STATE_OIL_TEMP:
            //no
            break;
        case MFA_STATE_AIR_TEMP:
            //no
            break;
#else

#endif
        default:
            break;
    }
}

// Simulate short press on MFA sensor
void pressMFASensorShort() {
    pressMFAMode();
}

// Simulate long press on MFA sensor
void pressMFASensorLong() {
    pressMFABlock();
}

// Simulate super long press on MFA sensor
void pressMFASensorSuperLong() {
    pressMFAReset();
}

// Simulate super super long press on MFA sensor
void pressMFASensorSuperSuperLong() {
    if (uptimeDisplayEnabled==0)
        uptimeDisplayEnabled=1;
    else  
        uptimeDisplayEnabled=0;
}
