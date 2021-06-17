#include "tacho.h"

uint32_t mRPMSenseData;
uint32_t lastMillisRPM;

void PCInt20()
{
    mRPMSenseData = (micros()-lastMillisRPM);
    lastMillisRPM = micros();
    //Serial.println(millis());
}

void initTacho()
{
    pinMode(RPM_PIN, INPUT_PULLUP);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(RPM_PIN), PCInt20, RISING);
    sei(); 
    lastMillisRPM = micros();
    mRPMSenseData = 0;
}

uint16_t readLastRPM()
{
    //Serial.println(mRPMSenseData);
    if (mRPMSenseData<1000000)
      return mRPMSenseData;
    else
      return 0;
}
