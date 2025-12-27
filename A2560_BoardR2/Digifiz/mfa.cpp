#include "mfa.h"
#include "display.h"
#include "orig_display.h"
#include "lcd_display.h"
#include "audi_display.h"
#include "audi_red_display.h"

uint8_t mfaMemorySelected = 0;
uint8_t prevMFAMode = HIGH;
uint8_t prevMFABlock = HIGH;
uint8_t prevMFAReset = HIGH;
uint8_t prevMFASensor = LOW;
uint8_t sensorPressed = 0;
uint32_t pressSensorTime = 0;

uint8_t uptimeDisplayEnabled = 0;

#ifdef EMULATE_RTC
extern RTC_Millis myRTC;
#else
extern RTC_DS3231 myRTC;
#endif

extern DateTime startTime[2];

extern digifiz_pars digifiz_parameters;

void initMFA()
{
    pinMode(MFA_MODE_PIN,INPUT);
    pinMode(MFA_BLOCK_PIN,INPUT);
    pinMode(MFA_RESET_PIN,INPUT);
    pinMode(MFA_SENSOR_PIN,INPUT);
    pressSensorTime = millis();
}

void processMFA()
{
    // Block MFA inputs shortly after boot to avoid unintended actions
    if (millis() < 3000)
    {
        prevMFAMode = digitalRead(MFA_MODE_PIN);
        prevMFABlock = digitalRead(MFA_BLOCK_PIN);
        prevMFAReset = digitalRead(MFA_RESET_PIN);
        prevMFASensor = digitalRead(MFA_SENSOR_PIN);
        return;
    }

#ifndef DISABLE_MANUFACTURER_MFA
    if (digifiz_parameters.option_mfa_manufacturer.value)
    {
      if ((digitalRead(MFA_MODE_PIN)==LOW)&&(prevMFAMode==HIGH))
      {
          //Pressed MFA Mode
          pressMFAMode();
      } 
      //digifiz_parameters.mfaBlock = (digitalRead(MFA_BLOCK_PIN)==LOW) ? 1 : 0;
      digifiz_parameters.mfaBlock.value = (digitalRead(MFA_BLOCK_PIN)==HIGH) ? 1 : 0;
      
      if ((digitalRead(MFA_RESET_PIN)==LOW)&&(prevMFAReset==HIGH))
      {
          //Pressed MFA Reset
          pressMFAReset();
      }
    }
#endif
#ifndef DISABLE_SENSOR_BUTTON
    if (digifiz_parameters.signalOptions_enable_touch_sensor.value)
    {
        if ((digitalRead(MFA_SENSOR_PIN)==HIGH)&&(prevMFASensor==LOW))
        {
            //Pressed MFA Sensor(on Digifiz)
            sensorPressed = 1;
            pressSensorTime = millis();
        }
        
        if ((digitalRead(MFA_SENSOR_PIN)==LOW) //logic is inversed!
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
    }
    
#endif
    prevMFAMode = digitalRead(MFA_MODE_PIN);
    prevMFABlock = digitalRead(MFA_BLOCK_PIN);
    prevMFAReset = digitalRead(MFA_RESET_PIN);
    prevMFASensor = digitalRead(MFA_SENSOR_PIN);
}

void pressMFASensorShort()
{
    pressMFAMode();
}

void pressMFASensorLong()
{
    pressMFABlock();
}

void pressMFASensorSuperLong()
{
    pressMFAReset();
}

void pressMFASensorSuperSuperLong()
{
  if (uptimeDisplayEnabled==0)
    uptimeDisplayEnabled=1;
  else  
    uptimeDisplayEnabled=0;
}

void pressMFAMode()
{
digifiz_parameters.mfaState.value+=1;  
#ifdef AUDI_RED_DISPLAY
    if (digifiz_parameters.mfaState.value>6) // 0 1 2 3 4 5 6
        digifiz_parameters.mfaState.value = 0;
#else
    if (digifiz_parameters.mfaState.value>5) // 0 1 2 3 4 5
        digifiz_parameters.mfaState.value = 0;
#endif
#ifdef SAVE_BY_MFA_MODE
    saveParameters();
#endif
}

void pressMFABlock()
{
    digifiz_parameters.mfaBlock.value+=1;
    digifiz_parameters.mfaBlock.value&=0x1;
    //saveParameters();
    //setMFABlock(digifiz_parameters.mfaBlock.value); //in display h
}

void pressMFAReset()
{
    switch(digifiz_parameters.mfaState.value)
    {
        case MFA_STATE_TRIP_DURATION:
            if (digifiz_parameters.mfaBlock.value)
            {
              digifiz_parameters.duration_1.value = 0;
              startTime[1] = myRTC.now();
            }
            else
            {
              digifiz_parameters.duration_0.value = 0;
              startTime[0] = myRTC.now();
            }
            break;
        case MFA_STATE_TRIP_DISTANCE:
            if (digifiz_parameters.mfaBlock.value)
            {
              digifiz_parameters.daily_mileage_1.value = 0;
            }
            else
            {
              digifiz_parameters.daily_mileage_0.value = 0;
            }
            break;
        case MFA_STATE_TRIP_L100KM:
            if (digifiz_parameters.mfaBlock.value)
            {
              digifiz_parameters.averageConsumption_1.value = 0;
            }
            else
            {
              digifiz_parameters.averageConsumption_0.value = 0;
            }
            //averageConsumption(digifiz_parameters.mfaBlock.value) = 0;
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            if (digifiz_parameters.mfaBlock.value)
            {
                digifiz_parameters.averageSpeed_1.value = 0;
            }
            else
            {
                digifiz_parameters.averageSpeed_0.value = 0;
            }
            //averageSpeed(digifiz_parameters.mfaBlock.value) = 0;
            break;
#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY) && !defined(TRANSPORTER_DISPLAY)
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
