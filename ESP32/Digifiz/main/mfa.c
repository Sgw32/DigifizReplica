#include "mfa.h"
#include "esp_log.h"
#include "millis.h"
#include "reg_inout.h"
#include "driver/gpio.h"
#include "adc.h"
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
    if (digifiz_parameters.signalOptions_enable_touch_sensor.value)
    {
        bMFASensor = gpio_get_level(TOUCH_PIN);
    }
    else
    {
        bMFASensor = 0;//gpio_get_level(TOUCH_PIN);
    }

    // Block MFA input actions shortly after boot to prevent spurious events
    if (millis() < 3000)
    {
        prevMFAMode = bMFAMode;
        prevMFABlock = bMFABlock;
        prevMFAReset = bMFAReset;
        prevMFASensor = bMFASensor;
        return;
    }

#ifndef DISABLE_MANUFACTURER_MFA
    if (digifiz_parameters.option_mfa_manufacturer.value)
    {
      if ((bMFAMode==0)&&(prevMFAMode==1))
      {
          //Pressed MFA Mode
          ESP_LOGI(LOG_TAG,  "Mode=1");
          pressMFAMode();
      } 
      digifiz_parameters.mfaBlock.value = (bMFABlock==1) ? 1 : 0;
      
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
    digifiz_parameters.mfaState.value+=1;  
#ifdef AUDI_RED_DISPLAY
    if (digifiz_parameters.mfaState.value>6) // 0 1 2 3 4 5 6
        digifiz_parameters.mfaState.value = 0;
#else
    if (digifiz_parameters.mfaState.value>5) // 0 1 2 3 4 5
        digifiz_parameters.mfaState.value = 0;
#endif
    saveParameters();
}

// Simulate pressing MFA block button
void pressMFABlock() {
    digifiz_parameters.mfaBlock.value+=1;
    digifiz_parameters.mfaBlock.value&=0x1;
}

// Simulate pressing MFA reset button
void pressMFAReset() {
    time_t current_time_t;
    time(&current_time_t);
    switch(digifiz_parameters.mfaState.value)
    {
        case MFA_STATE_TRIP_DURATION:
            digifiz_status.duration[digifiz_parameters.mfaBlock.value] = 0;
            if (digifiz_parameters.mfaBlock.value==0)
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
            digifiz_status.daily_mileage[digifiz_parameters.mfaBlock.value] = 0;
            break;
        case MFA_STATE_SENSOR:
            digifiz_status.averageConsumption[digifiz_parameters.mfaBlock.value] = 0;
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            digifiz_status.averageSpeed[digifiz_parameters.mfaBlock.value] = 0;
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


float getMFASensorValue() {
    switch (digifiz_parameters.mfa_sensor.value) {
        case 1:
            return getBarometerPressure();
        case 2:
            return getWidebandLambdaAFR();
        case 3:
            return getFuelPressure();
        case 0:
        default:
            return getFuelConsumption();
    }
}
