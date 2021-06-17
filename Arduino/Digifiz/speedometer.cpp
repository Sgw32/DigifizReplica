#include "speedometer.h"

uint16_t mSpdData;
uint32_t lastMillis;

void PCInt12()
{
    mSpdData = (micros()-lastMillis);
    lastMillis = micros();
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
}

uint16_t readLastSpeed()
{
    if (mSpdData<1000000)
      return mSpdData;
    else
      return 0;
}
