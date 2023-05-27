  #include "audi_display.h"


int mfa=0;
int mileage = 0;
void setup() 
{  
  initDisplay(); //Start MAX7219 display driver
  setRefuelSign(true);
  setCheckEngine(true);
    setBrightness(1);
  mfa=0;
  mileage = 0;
  setBacklight(true);
}

void loop() 
{
  setMFAType(mfa);
  mfa++;
  if (mfa==4)
    mfa=0;
  delay(100);
  setMileage(mileage);
  setDailyMileage(mileage);
  setAuxDigit(mileage);
  setBrightness(mileage&0xF);
  setFuel(32);
  setMFAClockData((mileage/100)%100,56);
  setRPMData((mileage*100)%7000);
  mileage++;
  setSpeedometerData(mileage&0xFF);
  //setCoolantData(mileage%18);
  setCoolantData(mileage%18);
  setBarData(17-mileage%18);
  
  //setBarData(17);
  /*setAll(0);
  delay(100);
  setAll(1);
  delay(100);*/
  
}
