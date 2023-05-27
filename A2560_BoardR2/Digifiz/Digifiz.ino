#include "display.h"  
#include "orig_display.h"
#include "lcd_display.h"
#include "speedometer.h"
#include "emergency.h"
#include "tacho.h"
#include "adc.h"
#include "ext_eeprom.h"
#include "protocol.h"
#include "buzzer.h"
#include "mfa.h"
#include "setup.h"
#include "audi_display.h"

//#include <DS3231.h>
#include <RTClib.h>
#include <Wire.h>

//Clock at 0x57
//EEPROM at 0x50
//DS3231 clock;

#ifdef DIGIFIZ_ORIGINAL_DISPLAY
extern uint8_t tr_status;
#endif

#ifdef DIGIFIZ_LCD_DISPLAY
extern uint8_t tr_status;
#endif

int i = 0;
int saveParametersCounter = 0;
uint16_t displaySpeedCnt = 0;
//Speed related data
uint32_t spd_m = 0;
float spd_m_speedometer = 0;
int spd_m_speedometerCnt = 0; //spd_m_speedometerCnt
float current_averageSpeed = 0;

//RPM-related data
float averageRPM = 0;
int averageRPMCnt = 0;
uint32_t rpm = 0;

//Time-related variables
#ifdef EMULATE_RTC
RTC_Millis myRTC;
#else
RTC_DS3231 myRTC;
RTC_Millis myRTC_doubled;
#endif
long clockDot;
int test = 1;
DateTime startTime[2];
int current_hour = 99;
int current_minute = 99;
DateTime newTime;
TimeSpan sinceStart = 0;
bool century = false;
bool h12Flag;
bool pmFlag;
bool clockRunning;

extern uint8_t uptimeDisplayEnabled;
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
  spd_m_speedometer = 0;
  spd_m_speedometerCnt = 0;
  averageRPM = 0;
  averageRPMCnt = 0;

  #ifdef USE_DISPLAY_LEDS
  pinMode(GPIO1_TO_GND_PIN,OUTPUT);
  pinMode(GPIO2_TO_GND_PIN,OUTPUT);
  digitalWrite(GPIO1_TO_GND_PIN,HIGH);
  digitalWrite(GPIO2_TO_GND_PIN,HIGH);
  #endif
  
  delay(100);
  Wire.begin(); // Start the I2C interface
  
  #ifdef EMULATE_RTC
  myRTC.begin(DateTime(F(__DATE__), F(__TIME__)));
  #else
  clockRunning = myRTC.begin();
  #endif
  
  initEEPROM(); //Start memory container
  current_averageSpeed = digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock];


  if (clockRunning)
  {
    startTime[0] = myRTC.now();
    startTime[0] = startTime[0] - TimeSpan(digifiz_parameters.duration[0]*60); //minus minutes
    startTime[1] = myRTC.now();
    startTime[1] = startTime[1] - TimeSpan(digifiz_parameters.duration[1]*60); //minus minutes
  }

  initDisplay(); //Start MAX7219 display driver

  
  initADC(); //Init ADC ports for 
  initSpeedometer();
  initTacho();
  initComProtocol();
  initBuzzer();
  initMFA();
  initEmergencyModule();
  clockDot = millis();
  
  initReadInterrupt();

  
  //setSpeedometerData(321);
  //delay(1000);
}

ISR(TIMER4_COMPA_vect)
{
  i=i+1;
  spd_m = readLastSpeed();
  //spd_m = 10000;
  if (spd_m>0)
  {
    spd_m = 1000000/spd_m ; //Hz
    spd_m *= digifiz_parameters.speedCoefficient; //to kmh (or to miles? - why not)
    #ifdef MILES
    spd_m *= 0.6214;
    #endif
    spd_m /= 100;
  }
#ifndef TESTMODE
  spd_m_speedometer += (spd_m-spd_m_speedometer)*0.5;
#endif

  //For test fuel intake
  //spd_m_speedometer = 60.0f;

  rpm = readLastRPM(); 
  if (rpm>0)
  {
    if((getRPMDispertion()<digifiz_parameters.medianDispFilterThreshold)) //30 or LESS!!!
    {
    rpm = 1000000/rpm;
    rpm *= digifiz_parameters.rpmCoefficient/100; //4 cylinder motor, 60 sec in min
    averageRPM += (rpm-averageRPM)*0.2;
    }
  }
  else
  {
    averageRPM += (0-averageRPM)*0.5;
  }

  //For test fuel intake
  #ifdef TESTMODE
  averageRPM = 3000.0f;
  #endif
    
  if (getBuzzerEnabled())
  {
      buzzerToggle();
  }
  processGasLevel();
  processCoolantTemperature();
  processOilTemperature();
  processAmbientTemperature();
  processBrightnessLevel();
  displaySpeedCnt++;
  if (displaySpeedCnt==4) // 2 Hz loop(as on original Digifiz)  
  {
    //setSpeedometerData(getCurrentMemoryBlock());
    
    setSpeedometerData((uint16_t)spd_m_speedometer);
    //setSpeedometerData(getBrightnessLevel());
    current_averageSpeed += (spd_m_speedometer-current_averageSpeed)*0.01;
#ifdef TESTMODE
  spd_m_speedometer+=1;
  if (spd_m_speedometer==25)
    spd_m_speedometer=0;
#endif
    displaySpeedCnt = 0;
  }
  //setSpeedometerData(getRawBrightnessLevel());
  setRPMData(averageRPM);
  #ifndef GALLONS
  uint8_t fuel = getLitresInTank();
  if (fuel<10)
    setRefuelSign(true);
  else
    setRefuelSign(false);
  #else
  uint8_t fuel = getGallonsInTank();
  if (fuel<2)
    setRefuelSign(true);
  else
    setRefuelSign(false);
  #endif
  setFuel(fuel);
  #if !defined(DIGIFIZ_ORIGINAL_DISPLAY) && !defined(DIGIFIZ_LCD_DISPLAY)
    setCoolantData(getDisplayedCoolantTemp());
  #else
    setCoolantData(getDisplayedCoolantTempOrig());
      if (!(tr_status&0x80))
      tr_status|=0x80;
    //fireDigifiz();
  #endif  
}

void loop() 
{
  #ifdef DIGIFIZ_LCD_DISPLAY
  processLCDIndicators();
  fireDigifiz();
  #endif
  #ifdef AUDI_DISPLAY
  fireDigifiz();
  #endif
  if ((millis()-clockDot)>500)
  {
      setDot(true);
      
  }
  if ((millis()-clockDot)>1000)
  {
    clockDot = millis();
    if (clockRunning)
    {
      newTime = myRTC.now();
      current_hour = newTime.hour();
      current_minute = newTime.minute();
      sinceStart = newTime - startTime[digifiz_parameters.mfaBlock];
      digifiz_parameters.duration[digifiz_parameters.mfaBlock] = sinceStart.totalseconds()/60;
      setClockData(current_hour,current_minute);
    }
    else
    {
      setClockData(99,99);
    }

    if (averageRPM>200)
      digifiz_parameters.uptime += 1;
    
    digifiz_parameters.mileage+=spd_m;
    digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]+=spd_m;
    digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock] = getCurrentIntakeFuelConsumption();//getFuelConsumption()*digifiz_parameters.tankCapacity;
    setMileage(uptimeDisplayEnabled ? (digifiz_parameters.uptime/3600) : (digifiz_parameters.mileage/3600)); //to km
    #ifdef AUDI_DISPLAY
    setDailyMileage((uint16_t)(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]/3600));
    #endif
    #ifndef YELLOW_GREEN_LED
    setBrightness(digifiz_parameters.autoBrightness ? getBrightnessLevel() : digifiz_parameters.brightnessLevel);
    #else
    setBrightness(digifiz_parameters.autoBrightness ? (getBrightnessLevel()+4) : digifiz_parameters.brightnessLevel);
    #endif
    
    saveParametersCounter++;
    setBacklight(digifiz_parameters.backlight_on ? true : false);
    //setAudiOptions(0x9);
    if (saveParametersCounter==EEPROM_SAVE_INTERVAL)
    {
        digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock] = current_averageSpeed;
        saveParameters();
        saveParametersCounter=0;
        //pressMFAMode();
        //setAudiOptions(0x6);
    }
    
    checkEmergency(rpm);
    setMFABlock(digifiz_parameters.mfaBlock ? 0 : 1); //in display h
    displayMFAType(digifiz_parameters.mfaState);
    setDot(false);
    
    
  }
  setMFAType(digifiz_parameters.mfaState);
  processMFA();
  protocolParse();
}
