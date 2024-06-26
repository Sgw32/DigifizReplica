#include "tacho.h"
#include "setup.h"

uint32_t mRPMSenseData;
uint32_t lastMillisRPM;

MedianDispertionFilter<uint32_t> medianFilterRPM(5);

/**
 * @brief PCInt20 interrupt which reads RPM
 * 
 */
void PCInt20()
{
    // mRPMSenseData should correspond 0..9000 RPM
    // Not lower(obvious) not higher(I assume)
    // 9000 RPM = 150 Hz , but for 1 cylinder. For 4 it is 300 Hz
    // So mRPMSenseData has a window of 1000000/300 ... 1000000
    uint32_t cur_micros = micros();
    uint32_t delta = (cur_micros-lastMillisRPM);
    #if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
    if (delta>3000)
    #else
    if (delta>1500)
    #endif
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

uint32_t getRPMDispertion()
{
  return medianFilterRPM.GetDispertion()>>12;
}

uint32_t getRPMMean()
{
  return medianFilterRPM.GetMean()>>2;
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
