#include "display.h"
#include "speedometer.h"
#include "tacho.h"
#include "adc.h"
//#include "eeprom.h"
#include <DS3231.h>
#include <Wire.h>

//Clock at 0x57
//EEPROM at 0x50
DS3231 clock; 

uint16_t rpm = 0;
uint16_t spd_m = 0;
int i = 0;
bool century = false;
bool h12Flag;
bool pmFlag;

long clockDot;

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
  //for (int i=0; i<5; i++){
      //delay(1000);
      Serial.print(clock.getYear(), DEC);
      Serial.print("-");
      Serial.print(clock.getMonth(century), DEC);
      Serial.print("-");
      Serial.print(clock.getDate(), DEC);
      Serial.print(" ");
      Serial.print(clock.getHour(h12Flag, pmFlag), DEC); //24-hr
      Serial.print(":");
      Serial.print(clock.getMinute(), DEC);
      Serial.print(":");
      Serial.println(clock.getSecond(), DEC);
  //}
  
  initDisplay();
  initADC();
  initSpeedometer();
  initTacho();
  
  Serial.begin(57600);
  
  Serial.println("hello");

  initReadInterrupt();

  clockDot = millis();
  
  //PRINTS("\nDigifiz Started!");
}

ISR(TIMER4_COMPA_vect)
{
  processGasLevel();
  processCoolantTemperature();
}

void loop() {
  
  if ((millis()-clockDot)>500)
  {
    setDot(true);
  }
  if ((millis()-clockDot)>1000)
  {
    int hour = clock.getHour(h12Flag, pmFlag);
    int minute = clock.getMinute();
    clockDot = millis();
    setClockData(hour,minute);
    setMFAClockData(hour,minute);
    setDot(false);
  }

  setSpeedometerData(spd_m);
  setRPMData(rpm);
  setFuel(getLitresInTank());
  //setFuel(constrain((int)getCoolantTemperature(),0,99));
  setCoolantData(getDisplayedCoolantTemp());
  i=i+1;

  spd_m = readLastSpeed();
  if (spd_m>0)
  {
    spd_m = 1000000/spd_m ; //Hz
  }
  
  rpm = readLastRPM(); //micros
  if (rpm>0)
  {
    rpm = 1000000/rpm;
    rpm *= 15; //4 cylinder motor, 60 sec in min
  }
  Serial.println(rpm,DEC);
  Serial.println(spd_m,DEC);
  delay(100);
}
