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
//#define EMULATE_RTC

#ifdef EMULATE_RTC
RTC_Millis myRTC;
#else
RTC_DS3231 myRTC;
#endif


uint32_t rpm = 0;
uint32_t spd_m = 0;
float spd_m_speedometer = 0;
int i = 0;
int saveParametersCounter = 0;
bool century = false;
bool h12Flag;
bool pmFlag;

float current_averageSpeed = 0;

long clockDot;
DateTime startTime[2];

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
  #ifdef EMULATE_RTC
  myRTC.begin(DateTime(F(__DATE__), F(__TIME__)));
  #else
  myRTC.begin();
  #endif
  startTime[0] = myRTC.now();
  startTime[0] = startTime[0] - TimeSpan(digifiz_parameters.duration[0]*60); //minus minutes
  startTime[1] = myRTC.now();
  startTime[1] = startTime[1] - TimeSpan(digifiz_parameters.duration[0]*60); //minus minutes
  current_averageSpeed = digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock];
  spd_m_speedometer = 0;
}

ISR(TIMER4_COMPA_vect)
{
  i=i+1;
  spd_m = readLastSpeed();
  if (spd_m>0)
  {
    spd_m = 1000000/spd_m ; //Hz
    spd_m *= digifiz_parameters.speedCoefficient; //to kmh (or to miles? - why not)
    spd_m /= 100;
    current_averageSpeed += (spd_m-current_averageSpeed)*0.001;
    spd_m_speedometer += (spd_m-spd_m_speedometer)*0.75;
  }
  rpm = readLastRPM(); //micros
  if (rpm>0)
  {
    rpm = 1000000/rpm;
    //rpm *= 15;
    rpm *= digifiz_parameters.rpmCoefficient/100; //4 cylinder motor, 60 sec in min
    //rpm /= 100; //TODO tests
  }
  if (getBuzzerEnabled())
  {
      buzzerToggle();
  }
  processGasLevel();
  processCoolantTemperature();
  processOilTemperature();
  processAmbientTemperature();
  setSpeedometerData((uint16_t)spd_m_speedometer);
  setRPMData(rpm);
  uint8_t fuel = getLitresInTank();
  if (fuel<10)
    setRefuelSign(true);
  else
    setRefuelSign(false);
  setFuel(fuel);
  setCoolantData(getDisplayedCoolantTemp());
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
    digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]+=spd_m;
    
    setMileage(digifiz_parameters.mileage/3600); //to km
    setBrightness(digifiz_parameters.brightnessLevel);
    saveParametersCounter++;
    setBacklight(digifiz_parameters.backlight_on ? true : false);
    if (saveParametersCounter==16)
    {
        digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock] = current_averageSpeed;
        digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock] = getFuelConsumption()*digifiz_parameters.tankCapacity;
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
