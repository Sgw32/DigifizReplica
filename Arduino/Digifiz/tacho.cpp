#include "tacho.h"

uint32_t mRPMSenseData;
uint32_t lastMillisRPM;

MedianFilter2<uint32_t> medianFilterRPM(5);

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
    medianFilterRPM.AddValue(0);
    medianFilterRPM.AddValue(0);
    medianFilterRPM.AddValue(0);
    medianFilterRPM.AddValue(0);
    medianFilterRPM.AddValue(0);
}

uint32_t readLastRPM()
{
    if ((micros()-lastMillisRPM)<1000000)
    {
      uint32_t median = medianFilterRPM.AddValue(mRPMSenseData);
      return median;
    }
    else
      return 0;
}
