#include "display.h"
#include "speedometer.h"
#include "tacho.h"
#include "adc.h"
#include "eeprom.h"
#include "protocol.h"

//#include <DS3231.h>
#include <RTClib.h>
#include <Wire.h>

//Clock at 0x57
//EEPROM at 0x50
//DS3231 clock; 
RTC_DS3231 myRTC;

uint16_t rpm = 0;
uint32_t spd_m = 0;
int i = 0;
bool century = false;
bool h12Flag;
bool pmFlag;

long clockDot;
DateTime startTime;

extern digifiz_pars digifiz_parameters;

void initReadInterrupt()
{
    cli();//stop interrupts
    //set timer4 interrupt at 1Hz
    TCCR4A = 0;// set entire TCCR1A register to 0
    TCCR4B = 0;// same for TCCR1B
    TCNT4  = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR4A = 1953;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR4B |= (1 << WGM12);
    // Set CS12 and CS10 bits for 1024 prescaler
    TCCR4B |= (1 << CS12) | (1 << CS10);  
    // enable timer compare interrupt
    TIMSK4 |= (1 << OCIE4A);
    sei();//allow interrupts
}

void setup() 
{  
  // Start the I2C interface
  Wire.begin();
  initEEPROM();
  initDisplay();
  initADC();
  initSpeedometer();
  initTacho();
  initReadInterrupt();
  initComProtocol();
  clockDot = millis();
  myRTC.begin();
  startTime = myRTC.now();
}

ISR(TIMER4_COMPA_vect)
{
  processGasLevel();
  processCoolantTemperature();
  setSpeedometerData(spd_m);
  setRPMData(rpm);
  setFuel(getLitresInTank());
  setCoolantData(getDisplayedCoolantTemp());
}

void loop() 
{
  
  if ((millis()-clockDot)>500)
  {
    setDot(true);
  }
  if ((millis()-clockDot)>1000)
  {
    DateTime newTime = myRTC.now();
    int hour = newTime.hour();
    int minute = newTime.minute();
    TimeSpan sinceStart = newTime - startTime;
    clockDot = millis();
    setClockData(hour,minute);
    setMFAClockData(sinceStart.hours(),sinceStart.minutes());
    setDot(false);
    digifiz_parameters.mileage+=spd_m;
    digifiz_parameters.daily_mileage+=spd_m;
    setMileage(digifiz_parameters.mileage/3600); //to km
    setMFAType(11);
  }
  
  protocolParse();
  i=i+1;
  spd_m = readLastSpeed();
  if (spd_m>0)
  {
    spd_m = 1000000/spd_m ; //Hz
    spd_m *= digifiz_parameters.speedCoefficient; //to kmh (or to miles? - why not)
  }
  rpm = readLastRPM(); //micros
  if (rpm>0)
  {
    rpm = 1000000/rpm;
    rpm *= digifiz_parameters.rpmCoefficient; //4 cylinder motor, 60 sec in min
  }
}
