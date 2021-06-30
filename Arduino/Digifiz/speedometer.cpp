#include "speedometer.h"

uint32_t mSpdData;
uint32_t lastMillis;

MedianFilter2<uint32_t> medianFilter(5);

void PCInt12()
{
    // mSpdData should correspond 0..300 kmh 
    // Not lower(obvious) not higher(of course)
    // 300 kmh ~ 300 Hz , we shift it to 2 times so it is 600 Hz(as in tacho)
    // So mRPMSenseData has a window of 1000000/600 ... 1000000
    uint32_t cur_micros = micros();
    uint32_t delta = (cur_micros-lastMillis);
    if (delta>1666)
    {
        mSpdData = (micros()-lastMillis);
        lastMillis = micros();
    }
    //Serial.println(millis());
}

void initSpeedometer()
{
    //pinMode(SPD_M_PIN, INPUT_PULLUP);
    DDRJ &= ~(1<<SPD_M_PIN);
    //DDRJ |= (1<<SPD_M_PIN);
    //PORTJ &= ~(1<<SPD_M_PIN);
    //PORTJ
    attachPinChangeInterrupt(12, PCInt12, RISING);
    sei(); 
    lastMillis = micros();
    mSpdData = 0;
    medianFilter.AddValue(0);
    medianFilter.AddValue(0);
    medianFilter.AddValue(0);
    medianFilter.AddValue(0);
    medianFilter.AddValue(0);
}

uint32_t readLastSpeed()
{
    if ((micros()-lastMillis)<1000000)
    {
      uint32_t median = medianFilter.AddValue(mSpdData);
      return median;
    }
    else
      return 0;
}
