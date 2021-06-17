#include "display.h"
#include "speedometer.h"
#include "tacho.h"
//#include "eeprom.h"
#include <DS3231.h>
#include <Wire.h>

DS3231 clock;

uint16_t rpm = 0;
uint16_t spd_m = 0;
int i = 0;
bool century = false;
bool h12Flag;
bool pmFlag;

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
  initSpeedometer();
  initTacho();
  
  Serial.begin(57600);
  
  Serial.println("hello");
  
  //PRINTS("\nDigifiz Started!");
  
  /*pinMode(CLK_PIN,OUTPUT);
  pinMode(DATA_PIN,OUTPUT);
  pinMode(CS_PIN,OUTPUT);
  pinMode(CLK_PIN2,OUTPUT);
  pinMode(DATA_PIN2,OUTPUT);
  pinMode(CS_PIN2,OUTPUT);*/
}

void loop() {
  int hour = clock.getHour(h12Flag, pmFlag);
  int minute = clock.getMinute();
  //int hour  = 0;
  //int minute = 0;
  // put your main code here, to run repeatedly:
  setClockData(hour,minute);
  setMFAClockData(hour,minute);
  setSpeedometerData(spd_m);
  setRPMData(rpm);
  setFuel(i);
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


 /*digitalWrite(CLK_PIN,HIGH);
  digitalWrite(DATA_PIN,HIGH);
  digitalWrite(CS_PIN,HIGH);
  digitalWrite(CLK_PIN2,HIGH);
  digitalWrite(DATA_PIN2,HIGH);
  digitalWrite(CS_PIN2,HIGH);
  delay(10);
  digitalWrite(CLK_PIN,LOW);
  digitalWrite(DATA_PIN,LOW);
  digitalWrite(CS_PIN,LOW);
  digitalWrite(CLK_PIN2,LOW);
  digitalWrite(DATA_PIN2,LOW);
  digitalWrite(CS_PIN2,LOW);
  delay(10);*/
}
