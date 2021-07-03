#include "mfa.h"
#include "display.h"

uint8_t mfaMemorySelected = 0;
uint8_t prevMFAMode = HIGH;
uint8_t prevMFABlock = HIGH;
uint8_t prevMFAReset = HIGH;
uint8_t prevMFASensor = LOW;
uint8_t sensorPressed = 0;
uint32_t pressSensorTime = 0;

extern RTC_DS3231 myRTC;
extern DateTime startTime;

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
  /*
    if ((digitalRead(MFA_MODE_PIN)==LOW)&&(prevMFAMode==HIGH))
    {
        //Pressed MFA Mode
        pressMFAMode();
    }
    if ((digitalRead(MFA_BLOCK_PIN)==LOW)&&(prevMFABlock==HIGH))
    {
        //Pressed MFA Block
        pressMFABlock();
    }
    if ((digitalRead(MFA_RESET_PIN)==LOW)&&(prevMFAReset==HIGH))
    {
        //Pressed MFA Reset
        pressMFAReset();
    }*/
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
    }
    
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

void pressMFAMode()
{
    digifiz_parameters.mfaState+=1;
    if (digifiz_parameters.mfaState>5)
        digifiz_parameters.mfaState = 0;
    saveParameters();
}

void pressMFABlock()
{
    digifiz_parameters.mfaBlock+=1;
    digifiz_parameters.mfaBlock&=0x1;
    //saveParameters();
    //setMFABlock(digifiz_parameters.mfaBlock); //in display h
}

void pressMFAReset()
{
    switch(digifiz_parameters.mfaState)
    {
        case MFA_STATE_TRIP_DURATION:
            digifiz_parameters.duration = 0;
            startTime = myRTC.now();
            break;
        case MFA_STATE_TRIP_DISTANCE:
            digifiz_parameters.daily_mileage = 0;
            break;
        case MFA_STATE_TRIP_L100KM:
            digifiz_parameters.averageConsumption = 0;
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            digifiz_parameters.averageSpeed = 0;
            break;
        case MFA_STATE_OIL_TEMP:
            //no
            break;
        case MFA_STATE_AIR_TEMP:
            //no
            break;
        default:
            break;
    }
}
