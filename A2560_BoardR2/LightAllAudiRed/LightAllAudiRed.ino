#include "audi_display.h"


int mfa=0;
int mileage = 0;
void setup() 
{  
  initDisplay(); //Start MAX7219 display driver
  setRefuelSign(true);
  setCheckEngine(true);
    setBrightness(15);
  mfa=0;
  mileage = 0;
  setBacklight(true);
  DDRJ &= ~(1<<PJ3);
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
  setMFAClockData(88,88);
  setClockData(88,88);
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
  
 /* delay(500);
  setAll(1);
  delay(500);*/
}
