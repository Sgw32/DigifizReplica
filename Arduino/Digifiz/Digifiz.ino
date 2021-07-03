#include "display.h"
#include "speedometer.h"
#include "emergency.h"
#include "tacho.h"
#include "adc.h"
#include "eeprom.h"
#include "protocol.h"
#include "buzzer.h"
#include "mfa.h"

//#include <DS3231.h>
#include <RTClib.h>
#include <Wire.h>

//Clock at 0x57
//EEPROM at 0x50
//DS3231 clock; 
RTC_DS3231 myRTC;

uint32_t rpm = 0;
uint32_t spd_m = 0;
int i = 0;
int saveParametersCounter = 0;
bool century = false;
bool h12Flag;
bool pmFlag;

float current_averageSpeed = 0;

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
  spd_m = 0;
  rpm = 0;
  
  Wire.begin(); // Start the I2C interface
  initEEPROM(); //Start memory container
  initDisplay(); //Start MAX7219 display driver
  initADC(); //Init ADC ports for 
  initSpeedometer();
  initTacho();
  initReadInterrupt();
  initComProtocol();
  initBuzzer();
  initMFA();
  initEmergencyModule();
  clockDot = millis();
  myRTC.begin();
  startTime = myRTC.now();
  startTime = startTime - TimeSpan(digifiz_parameters.duration*60); //minus minutes
  current_averageSpeed = digifiz_parameters.averageSpeed;
}

ISR(TIMER4_COMPA_vect)
{
  processGasLevel();
  processCoolantTemperature();
  setSpeedometerData(spd_m);
  setRPMData(rpm);
  setFuel(getLitresInTank());
  setCoolantData(getDisplayedCoolantTemp());
  i=i+1;
  spd_m = readLastSpeed();
  if (spd_m>0)
  {
    spd_m = 1000000/spd_m ; //Hz
    spd_m *= digifiz_parameters.speedCoefficient; //to kmh (or to miles? - why not)
    current_averageSpeed += (spd_m-current_averageSpeed)*0.001;
  }
  rpm = readLastRPM(); //micros
  if (rpm>0)
  {
    rpm = 1000000/rpm;
    rpm *= digifiz_parameters.rpmCoefficient; //4 cylinder motor, 60 sec in min
  }
  if (getBuzzerEnabled())
  {
      buzzerToggle();
  }
}

void loop() 
{
  if ((millis()-clockDot)>500)
  {
    //if (digifiz_parameters.mfaState==MFA_STATE_TRIP_DURATION)
    //{
      setDot(true);
    //}
  }
  if ((millis()-clockDot)>1000)
  {
    clockDot = millis();
    DateTime newTime = myRTC.now();
    int hour = newTime.hour();
    int minute = newTime.minute();
    setClockData(hour,minute);
    digifiz_parameters.mileage+=spd_m;
    digifiz_parameters.daily_mileage+=spd_m;
    setMileage(digifiz_parameters.mileage/3600); //to km
    saveParametersCounter++;
    if (saveParametersCounter==16)
    {
        digifiz_parameters.averageSpeed = current_averageSpeed;
        saveParameters();
        saveParametersCounter=0;
    }
    checkEmergency(rpm);
    setMFABlock(digifiz_parameters.mfaBlock); //in display h
    displayMFAType(digifiz_parameters.mfaState);
    setDot(false);
  }
  setMFAType(digifiz_parameters.mfaState);
  processMFA();
  protocolParse();
}
