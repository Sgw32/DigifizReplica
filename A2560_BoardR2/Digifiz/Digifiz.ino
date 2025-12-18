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
#include "audi_red_display.h"

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
uint8_t testMode = 0;
DateTime startTime[2];
int current_hour = 99;
int current_minute = 99;
uint8_t coolant_segments = 0;
DateTime newTime;
TimeSpan sinceStart = 0;
bool clockRunning;
uint8_t fuel = 0;

extern uint8_t uptimeDisplayEnabled;
extern digifiz_pars digifiz_parameters;

/**
 * @brief Sets up main interrupt of device, interrupt freq ~8 Hz
 * 
 */
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

/**
 * @brief Arduino (or Arduino-like) setup function
 * 
 */
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
  current_averageSpeed = averageSpeed(digifiz_parameters.mfaBlock.value);


  if (clockRunning)
  {
    startTime[0] = myRTC.now();
    startTime[0] = startTime[0] - TimeSpan(durationMinutes(0)*60); //minus minutes
    startTime[1] = myRTC.now();
    startTime[1] = startTime[1] - TimeSpan(durationMinutes(1)*60); //minus minutes
  }

  initDisplay(); //Start MAX7219 display driver

  
  initADC(); //Init ADC ports for 
  initSpeedometer();
  initTacho();
  initComProtocol();
  initBuzzer();
  initMFA();
  initEmergencyModule();
  #ifdef FUEL_PRESSURE_SENSOR
  initFuelPressureSensor();
  #endif
  clockDot = millis();
  
  initReadInterrupt();
  testMode = 0;
  
  //setSpeedometerData(321);
  //delay(1000);
}

/**
 * @brief Main interrupt of the device
 * 
 */
ISR(TIMER4_COMPA_vect)
{
  if (testMode)
  {
    if (displaySpeedCnt==3) // 2 Hz loop(as on original Digifiz)  
    {
      averageRPM+=50;
      if (averageRPM>8000)
        averageRPM = 0;
      spd_m_speedometer+=1;
      if (spd_m_speedometer>=300)
        spd_m_speedometer=0;
      fuel+=1;
      if (fuel>99)
        fuel=0;
      if (digifiz_parameters.option_gallons.value)
      {
        if (fuel<2)
          setRefuelSign(true);
        else
          setRefuelSign(false);
      }
      else
      {
        if (fuel<10)
          setRefuelSign(true);
        else
          setRefuelSign(false);  
      }
      coolant_segments+=1;
      if (coolant_segments>14)
        coolant_segments=0;
    }
  }
  else
  {
    spd_m = readLastSpeed();
    if (spd_m>0)
    {
      spd_m = 1000000/spd_m ; //Hz
      spd_m *= digifiz_parameters.speedCoefficient.value; //to kmh (or to miles? - why not)
      //#ifdef MILES
      if (digifiz_parameters.option_miles.value)
        spd_m *= 0.6214;
      //#endif
      spd_m /= 100;
    }
    spd_m_speedometer += (spd_m-spd_m_speedometer)*0.5;
    rpm = readLastRPM(); 
    if (rpm>0)
    {
      if((getRPMDispertion()<digifiz_parameters.medianDispFilterThreshold.value)) //30 or LESS!!!
      {
      rpm = 1000000/rpm;
      float rpm_quadratic_coeff = digifiz_parameters.rpmQuadraticCoefficient.value / 100000.0f;
      float rpm_linear_coeff = digifiz_parameters.rpmCoefficient.value / 100.0f; //4 cylinder motor, 60 sec in min
      float rpm_raw = rpm;
      rpm = (rpm_quadratic_coeff * rpm_raw * rpm_raw) + (rpm_linear_coeff * rpm_raw);
      averageRPM += (rpm-averageRPM)*digifiz_parameters.rpmFilterK.value/140; //default = 0.2
      }
    }
    else
    {
      averageRPM += (0-averageRPM)*0.5;
    }

    if (digifiz_parameters.option_gallons.value)
    {
      fuel = getGallonsInTank();
      if (fuel<2)
        setRefuelSign(true);
      else
        setRefuelSign(false);
    }
    else
    {
      fuel = getLitresInTank();
      if (fuel<10)
        setRefuelSign(true);
      else
        setRefuelSign(false);  
    }

#if !defined(DIGIFIZ_ORIGINAL_DISPLAY) && !defined(DIGIFIZ_LCD_DISPLAY)
    coolant_segments = getDisplayedCoolantTemp();
 #else
    coolant_segments = getDisplayedCoolantTempOrig();
#endif  
  }

  displaySpeedCnt++;
  if (displaySpeedCnt==4) // 2 Hz loop(as on original Digifiz)  
  {
    setSpeedometerData((uint16_t)spd_m_speedometer);
    current_averageSpeed += (spd_m_speedometer-current_averageSpeed)*0.01;
    displaySpeedCnt = 0;
  }
  if (getBuzzerEnabled())
  {
      buzzerToggle();
  }
  processGasLevel();
  processCoolantTemperature();
  processOilTemperature();
  processAmbientTemperature();
  processBrightnessLevel();
  #ifdef FUEL_PRESSURE_SENSOR
  processFuelPressure();
  #endif
  
  setRPMData(averageRPM);

  setFuel(fuel);

  #if !defined(DIGIFIZ_ORIGINAL_DISPLAY) && !defined(DIGIFIZ_LCD_DISPLAY)
    setCoolantData(coolant_segments);
  #else
    setCoolantData(coolant_segments);
      if (!(tr_status&0x80))
      tr_status|=0x80;
  #endif  
}

/**
 * @brief Arduino (or Arduino-like) loop function
 * 
 */
void loop() 
{
  #ifdef DIGIFIZ_LCD_DISPLAY
  processLCDIndicators();
  fireDigifiz();
  #else
  #if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
  fireDigifiz();
  #else
  fireDigifiz();
  #endif
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
      sinceStart = newTime - startTime[digifiz_parameters.mfaBlock.value];
      durationMinutes(digifiz_parameters.mfaBlock.value) = sinceStart.totalseconds()/60;
      setClockData(current_hour,current_minute);
    }
    else
    {
      setClockData(99,99);
    }

    if (averageRPM>200)
      digifiz_parameters.uptime.value += 1;
    
    digifiz_parameters.mileage.value+=spd_m;
    dailyMileage(digifiz_parameters.mfaBlock.value)+=spd_m;
    #ifdef AVERAGE_CONSUMPTION_L100KM
      averageConsumption(digifiz_parameters.mfaBlock.value) += 0.01f*(getCurrentIntakeFuelConsumption()-averageConsumption(digifiz_parameters.mfaBlock.value));//getFuelConsumption()*digifiz_parameters.tankCapacity;
    #endif

    #ifdef CURRENT_CONSUMPTION_L100KM
    averageConsumption(digifiz_parameters.mfaBlock.value) = getCurrentIntakeFuelConsumption();//getFuelConsumption()*digifiz_parameters.tankCapacity;
    #endif
    
    if (millis()<5000)
    {
      setMileage(getLoadResult());
    }
    else
    {
      setMileage(uptimeDisplayEnabled ? (digifiz_parameters.uptime.value/3600) : (digifiz_parameters.mileage.value/3600)); //to km
    }
    
    #if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
    setDailyMileage((uint16_t)(dailyMileage(digifiz_parameters.mfaBlock.value)/3600));
    #endif

    if (millis()>2000)
    {
    #ifndef YELLOW_GREEN_LED
    setBrightness(digifiz_parameters.autoBrightness.value ? getBrightnessLevel() : digifiz_parameters.brightnessLevel.value);
    #else
    setBrightness(digifiz_parameters.autoBrightness.value ? (getBrightnessLevel()+7) : digifiz_parameters.brightnessLevel.value);
    #endif
    }
    saveParametersCounter++;
    setBacklight(digifiz_parameters.backlight_on.value ? true : false);
    //setAudiOptions(0x9);
    if (saveParametersCounter>EEPROM_SAVE_INTERVAL)
    {
        averageSpeed(digifiz_parameters.mfaBlock.value) = current_averageSpeed;
        saveParameters();
        saveParametersCounter=0;
        //pressMFAMode();
        //setAudiOptions(0x6);
    }
    checkEmergency(rpm);
    setMFABlock(digifiz_parameters.mfaBlock.value ? 0 : 1); //in display h
    displayMFAType(uptimeDisplayEnabled ? 6 : digifiz_parameters.mfaState.value);
    setDot(false);
  }
  setMFAType(uptimeDisplayEnabled ? 6 : digifiz_parameters.mfaState.value);
  processMFA();
  protocolParse();
}
