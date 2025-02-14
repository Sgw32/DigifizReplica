#include "audi_display.h"


int mfa=0;
int mileage = 0;

#define OIL_0_3BAR_PIN A1 //PF1
#define OIL_1_8BAR_PIN A2 //PF2
#define CHECK_ENGINE_IN 65 //PJ2
#define OIL_LED_PIN 22 //PA0

void setup() 
{  
  initDisplay(); //Start MAX7219 display driver
  setRefuelSign(true);
  setCheckEngine(true);
    setBrightness(15);
  mfa=0;
  mileage = 0;
  setBacklight(true);
  pinMode(OIL_0_3BAR_PIN, INPUT);
  pinMode(OIL_1_8BAR_PIN, INPUT);
  //pinMode(CHECK_ENGINE_IN, INPUT);
  DDRJ&=~(1<<PJ2);
  pinMode(OIL_LED_PIN,OUTPUT);
  digitalWrite(OIL_LED_PIN, HIGH);
}

void loop() 
{
  mfa++;
  if (mfa==4)
    mfa=0;
  delay(100);
  setMileage(mileage);
    setMFAType(6);
  setAuxDigit(mileage);
  setFuel(88);
  setMFAClockData(12,34);
  setClockData(56,78);
  setRPMData(5000);
  mileage++;
  if (PINJ&(1<<PJ3))
    setSpeedometerData(0);
  else
    setSpeedometerData(1);
  //setCoolantData(mileage%18);
  setCoolantData(8);
  setBarData(5);
  fireDigifiz();
  setAll(0);
  setMFAClockData(12,34);
  setMFADisplayedNumber(1234);
  digitalWrite(OIL_LED_PIN, HIGH);
  
 /* delay(500);
  setAll(1);
  delay(500);*/
}
