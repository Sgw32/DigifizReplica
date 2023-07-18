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
  setFuel(66);
  setMFAClockData(00,12);
  setClockData(56,78);
  setRPMData(5000);
  mileage++;
  setSpeedometerData(mileage&0xFF);
  //setCoolantData(mileage%18);
  setCoolantData(16);
  setBarData(16);
  /*setAll(0);
  setMFAClockData(12,34);
  setMFADisplayedNumber(1234);
  delay(500);
  setAll(1);
  delay(500);*/
}
