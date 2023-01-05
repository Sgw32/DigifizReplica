#include "emergency.h"
#include "lcd_display.h"

uint8_t emergency_state = 0;
uint16_t emergencyRPM = 2500;
uint32_t emergencyCounter;
uint8_t last_emergency_state = 0;
void initEmergencyModule()
{
    pinMode(OIL_0_3BAR_PIN, INPUT);
    pinMode(OIL_1_8BAR_PIN, INPUT);
    //pinMode(CHECK_ENGINE_IN, INPUT);
    DDRJ&=~(1<<PJ2);
    pinMode(OIL_LED_PIN,OUTPUT);
    emergencyCounter = millis();
}

uint8_t processCHECKEngine()
{
    if (PINJ&(1<<PJ2))
    {
        setCheckEngine(false);
    }
    else
    {
        setCheckEngine(true);
    }
    return 0;
}

uint8_t processOilPressure(int mRPM)
{
    emergency_state = 0;
    if (digitalRead(OIL_0_3BAR_PIN)==LOW)
    {
        emergency_state = 1;
    }

#ifndef EMERGENCY_DISABLE_SENSOR_CHECK
    if (((last_emergency_state==0))&&
        (digitalRead(OIL_1_8BAR_PIN)==HIGH)
        &&(mRPM>emergencyRPM))  //Was a problem with Oil 1.8 bar sensor, and we reached RPM
    {
        emergency_state = 2;
        emergencyCounter = millis();
    }
   
    if (((millis()-emergencyCounter)>1000)&&(last_emergency_state==2)&&
        (digitalRead(OIL_1_8BAR_PIN)==HIGH)
        &&(mRPM>emergencyRPM))  //It is more than second we are at 2000 RPM and there is no pressure
    {
        //OIL 1.8 error!
        emergency_state = 3;
    }
#endif
    
    last_emergency_state = emergency_state;
    return emergency_state;
}

void checkEmergency(int mRPM)
{
    emergency_state = processOilPressure(mRPM);
    processCHECKEngine();
    if (emergency_state==0)
    {
        digitalWrite(OIL_LED_PIN,LOW);
        buzzerOff();
    }
    if (emergency_state==1)
    {
        digitalWrite(OIL_LED_PIN,HIGH); //emergency, but for 0.3 bar sensor only
        buzzerOff();
    }
    if (emergency_state==3)
    {
        digitalWrite(OIL_LED_PIN,HIGH); //emergency, pressure system
        buzzerOn();
    }
}
