#include "tacho.h"

uint32_t mRPMSenseData;
uint32_t lastMillisRPM;

MedianFilter2<uint32_t> medianFilterRPM(5);

void PCInt20()
{
    // mRPMSenseData should correspond 0..9000 RPM
    // Not lower(obvious) not higher(I assume)
    // 9000 RPM = 150 Hz , but for 1 cylinder. For 4 it is 300 Hz
    // So mRPMSenseData has a window of 1000000/300 ... 1000000
    uint32_t cur_micros = micros();
    uint32_t delta = (cur_micros-lastMillisRPM);
    if (delta>3000)
    {
        mRPMSenseData = delta;
        lastMillisRPM = micros();
    }
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
      //Constrain filtered input here
      uint32_t median = medianFilterRPM.AddValue(mRPMSenseData);
      return median;
    }
    else
      return 0;
}
