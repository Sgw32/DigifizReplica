#include "lcd_display.h"
#include <LX_Lcd_Vario.h>
#include <Wire.h>

#ifdef DIGIFIZ_LCD_DISPLAY
extern digifiz_pars digifiz_parameters;

#ifdef EMULATE_RTC
extern RTC_Millis myRTC;
#else
extern RTC_DS3231 myRTC;
#endif

extern DateTime startTime[2];
extern bool clockRunning;
int orig_mRPMData = 4000;
bool orig_floatDot = 0;
extern TimeSpan sinceStart;

uint8_t tr_status = 0x00; // sending, clockbit, 000000
uint16_t data_cnt = 0;

//create the object
LX_Lcd_Vario lx;
//Setup the Variables that you have to pass to the library, this works for the LX Vario
#define PCFAddress B111000 //This defines the Slave Adress on the I2C Bus, its controlled via the SA0 Pin
byte set_modeset = MODESET | MODE_NORMAL | DISPLAY_ENABLED | BIAS_THIRD | DRIVE_4;
byte set_blink = BLINK | BLINKING_ALTERNATION | BLINK_FREQUENCY_OFF;
byte set_datapointer = LOADDATAPOINTER | 0;
byte set_bankselect = BANKSELECT | BANKSELECT_O1_RAM0 | BANKSELECT_O2_RAM0; //doesn't really matter, because Drive mode is 4
byte set_deviceselect_1 = DEVICE_SELECT; //A0, A1, A2 set to ground
byte set_deviceselect_2 = DEVICE_SELECT|DEVICE_SELECT_A0; //A1, A2 set to ground, A0 high

//0 - RPM 7300-8000
//1 - RPM 6500-7200
//2 - RPM 5700-6400
//3 - RPM 4900-5600
//4 - RPM 4100-4800
//5 - RPM 3300-4000
//6 - RPM 2500-3200
//7 - RPM 1700-2400
//8 - RPM 900-1600
//9 - RPM 100-800
//10 - milage 1 + ind 1
//11 - milage 2 + ind 2
//12 - milage 3 + ind 3
//13 - milage 4 + ind 4
//14 - milage 5 + ind 5
//15 - milage 6(first part) + ind 6 + time 2 first part+ ind 8
//16 - time 2 second part + time 3 first part + L/100 MFA
//17 - time 3 second part + time 4 first part + KMH MFA
//18 - time 4 second part + mfa 1 first part + TIME MFA
//19 - mfa 1 second part + mfa 2 first part + KM MFA
//20 - mfa 2 second part + mfa 3 first part + OIL T MFA
//21 - mfa 3 second part + mfa 4 first part + AMB T MFA
//22 - mfa 4 second part + fuel 1 first part + ind 9
//23 - fuel 1 second part + fuel 2 first part + R common ind
//24 - fuel 2 second part + ind 10,11,12; 0x01 - MFA 2, 0x01 - Gen, 0x04 - MFA1
//25 - T coolant 1-8
//26 - T coolant 9-16
//27 - T coolant 17-24
//28 - SPEED 3
//29 - SPEED 2
//30 - SPEED 1
//31 - time 1 first part, check engine
//32 - time 2 first part, milage 6 second part




void initDisplayClockTimer()
{
   
}

void init_digifiz()
{
    tr_status|=0x20;
    delay(100); //time to wake up
    //add your controller to the Library.
    lx.addPCF(B111000, set_modeset, set_deviceselect_1, set_blink, set_bankselect);
    lx.addPCF(B111000, set_modeset, set_deviceselect_2, set_blink, set_bankselect);
    //setup the controller
    lx.init();
    lx.clear();

    lx.setDigifizBufferSegment(24, 0x02,1); //Battery
    //lx.setDigifizBufferSegment(24, 0x04,1); //MFA1
    //lx.setDigifizBufferSegment(24, 0x01,1); //MFA2
    lx.setDigifizBufferSegment(24, 0x08,1); //Oil
    lx.setDigifizBufferSegment(10, 0x80,1); //lights
    lx.setDigifizBufferSegment(11, 0x80,1); //heat lights
    lx.setDigifizBufferSegment(12, 0x80,1);  //back lights heat
    lx.setDigifizBufferSegment(13, 0x80,1);  // back window heat
    lx.setDigifizBufferSegment(14, 0x80,1); //main beam
    lx.setDigifizBufferSegment(15, 0x80,1); //turn left
    lx.setDigifizBufferSegment(22, 0x08,1); //brakes
    lx.setDigifizBufferSegment(15, 0x08,1); //turn right

    pinMode(MFA1_PIN,OUTPUT);
    pinMode(MFA2_PIN,OUTPUT);
    pinMode(BACKLIGHT_CTL_PIN,OUTPUT);
}

void initDisplay()
{
    init_digifiz();
}

void setRPM(int rpmdata)
{ 
    orig_mRPMData=rpmdata;
}

void setBacklight(bool onoff)
{
    digitalWrite(BACKLIGHT_CTL_PIN, onoff ? HIGH : LOW);
}

void blinking()
{

}

void blinking2()
{

}

void setMFABlock(uint8_t block)
{
  if (block&0x1)
  {
      digitalWrite(MFA1_PIN,HIGH);
      digitalWrite(MFA2_PIN,LOW);
      lx.setDigifizBufferSegment(24, 0x04,1); 
      lx.setDigifizBufferSegment(24, 0x01,0);
  }
  else
  {
      digitalWrite(MFA1_PIN,LOW);
      digitalWrite(MFA2_PIN,HIGH);
      lx.setDigifizBufferSegment(24, 0x04,0); 
      lx.setDigifizBufferSegment(24, 0x01,1); 
  }
}

void setRefuelSign(bool onoff)
{
    lx.setDigifizBufferSegment(23, 0x08,1);
}

void setCheckEngine(bool onoff)
{
    //lx.setDigifizBufferSegment(31, 0x08,onoff ? 1 : 0);
}

void displayMFAType(uint8_t mfaType)
{    
    switch(digifiz_parameters.mfaState)
    {
        case MFA_STATE_TRIP_DURATION:
            setMFAClockData(sinceStart.hours(),sinceStart.minutes());
            break;
        case MFA_STATE_TRIP_DISTANCE:
            setMFADisplayedNumber((uint16_t)digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]/3600);
            setFloatDot(false);
            break;
        case MFA_STATE_TRIP_L100KM:
            setMFADisplayedNumber((uint16_t)digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock]*100);
            setFloatDot(true);
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            setMFADisplayedNumber((uint16_t)fabs(digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock]*10));
            setFloatDot(true);
            break;
        case MFA_STATE_OIL_TEMP:
            #ifdef FAHRENHEIT
              setMFADisplayedNumber((int16_t)getOilTemperatureFahrenheit());
              setFloatDot(false);
            #else
              setMFADisplayedNumber((int16_t)(getOilTemperature()));
              setFloatDot(true);
            #endif
            break;
        case MFA_STATE_AIR_TEMP:
            #ifdef FAHRENHEIT
              setMFADisplayedNumber((int16_t)getAmbientTemperatureFahrenheit());
              setFloatDot(false);
            #else
              setMFADisplayedNumber((int16_t)getAmbientTemperature());
              setFloatDot(true);
            #endif
            
            break;
        default:
            break;
    }
}

void setMFAType(uint8_t type)
{
  if (type>6)
  {
      return;
  }  
  switch (type)
  {
    case 0:
      lx.setDigifizBufferSegment(17, 0x08, 0); //kmh
      lx.setDigifizBufferSegment(16, 0x08, 0); //l100
      lx.setDigifizBufferSegment(18, 0x08, 1); //dur
      lx.setDigifizBufferSegment(19, 0x08, 0); //km
      lx.setDigifizBufferSegment(20, 0x08, 0); //Oil T
      lx.setDigifizBufferSegment(21, 0x08, 0); //Amb T
      break;
    case 1:
      lx.setDigifizBufferSegment(17, 0x08, 0); //kmh
      lx.setDigifizBufferSegment(16, 0x08, 0); //l100
      lx.setDigifizBufferSegment(18, 0x08, 0); //dur
      lx.setDigifizBufferSegment(19, 0x08, 1); //km
      lx.setDigifizBufferSegment(20, 0x08, 0); //Oil T
      lx.setDigifizBufferSegment(21, 0x08, 0); //Amb T
      break;
    case 2:
      lx.setDigifizBufferSegment(17, 0x08, 0); //kmh
      lx.setDigifizBufferSegment(16, 0x08, 1); //l100
      lx.setDigifizBufferSegment(18, 0x08, 0); //dur
      lx.setDigifizBufferSegment(19, 0x08, 0); //km
      lx.setDigifizBufferSegment(20, 0x08, 0); //Oil T
      lx.setDigifizBufferSegment(21, 0x08, 0); //Amb T
      break;
    case 3:
      lx.setDigifizBufferSegment(17, 0x08, 1); //kmh
      lx.setDigifizBufferSegment(16, 0x08, 0); //l100
      lx.setDigifizBufferSegment(18, 0x08, 0); //dur
      lx.setDigifizBufferSegment(19, 0x08, 0); //km
      lx.setDigifizBufferSegment(20, 0x08, 0); //Oil T
      lx.setDigifizBufferSegment(21, 0x08, 0); //Amb T
      break;
    case 4:
      lx.setDigifizBufferSegment(17, 0x08, 0); //kmh
      lx.setDigifizBufferSegment(16, 0x08, 0); //l100
      lx.setDigifizBufferSegment(18, 0x08, 0); //dur
      lx.setDigifizBufferSegment(19, 0x08, 0); //km
      lx.setDigifizBufferSegment(20, 0x08, 1); //Oil T
      lx.setDigifizBufferSegment(21, 0x08, 0); //Amb T
      break;
    case 5:
      lx.setDigifizBufferSegment(17, 0x08, 0); //kmh
      lx.setDigifizBufferSegment(16, 0x08, 0); //l100
      lx.setDigifizBufferSegment(18, 0x08, 0); //dur
      lx.setDigifizBufferSegment(19, 0x08, 0); //km
      lx.setDigifizBufferSegment(20, 0x08, 0); //Oil T
      lx.setDigifizBufferSegment(21, 0x08, 1); //Amb T
      break;
    case 6:
      lx.setDigifizBufferSegment(17, 0x08, 0); //kmh
      lx.setDigifizBufferSegment(16, 0x08, 0); //l100
      lx.setDigifizBufferSegment(18, 0x08, 0); //dur
      lx.setDigifizBufferSegment(19, 0x08, 0); //km
      lx.setDigifizBufferSegment(20, 0x08, 0); //Oil T
      lx.setDigifizBufferSegment(21, 0x08, 0); //Amb T
      break;
    default:
      break;
  }
}

void setBrightness(uint8_t levels)
{
  
}

void setMileage(uint32_t mileage)
{
  uint8_t number[10]={0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00001110,0b01111111,0b01101111};
  //We set mileage but do not set indicators
  if (((mileage / 100000) % 10)!=0)
  {
      lx.setDigifizBufferMasked(10, number[(mileage / 100000) % 10], 0x7F);
      lx.setDigifizBufferMasked(11, number[(mileage / 10000) % 10], 0x7F);
      lx.setDigifizBufferMasked(12, number[(mileage / 1000) % 10], 0x7F);
      lx.setDigifizBufferMasked(13, number[(mileage / 100) % 10], 0x7F);
      lx.setDigifizBufferMasked(14, number[(mileage / 10) % 10], 0x7F);
      lx.setDigifizBufferMasked(15, number[(mileage / 1) % 10], 0x70);
      lx.setDigifizBufferMasked(32, number[(mileage / 1) % 10],0xF); 
  }
  else
  {
    if (((mileage / 10000) % 10)!=0)
    {
          lx.setDigifizBufferMasked(10, 0, 0x7F);
          lx.setDigifizBufferMasked(11, number[(mileage / 10000) % 10], 0x7F);
          lx.setDigifizBufferMasked(12, number[(mileage / 1000) % 10], 0x7F);
          lx.setDigifizBufferMasked(13, number[(mileage / 100) % 10], 0x7F);
          lx.setDigifizBufferMasked(14, number[(mileage / 10) % 10], 0x7F);
          lx.setDigifizBufferMasked(15, number[(mileage / 1) % 10], 0x70);
          lx.setDigifizBufferMasked(32, number[(mileage / 1) % 10],0xF); 
    }
    else
    {
      if (((mileage / 1000) % 10)!=0)
      {
          lx.setDigifizBufferMasked(10, 0, 0x7F);
          lx.setDigifizBufferMasked(11, 0, 0x7F);
          lx.setDigifizBufferMasked(12, number[(mileage / 1000) % 10], 0x7F);
          lx.setDigifizBufferMasked(13, number[(mileage / 100) % 10], 0x7F);
          lx.setDigifizBufferMasked(14, number[(mileage / 10) % 10], 0x7F);
          lx.setDigifizBufferMasked(15, number[(mileage / 1) % 10], 0x70);
          lx.setDigifizBufferMasked(32, number[(mileage / 1) % 10],0xF); 
      }
      else
      {
        if (((mileage / 100) % 10)!=0)
        {
              lx.setDigifizBufferMasked(10, 0, 0x7F);
              lx.setDigifizBufferMasked(11, 0, 0x7F);
              lx.setDigifizBufferMasked(12, 0, 0x7F);
              lx.setDigifizBufferMasked(13, number[(mileage / 100) % 10], 0x7F);
              lx.setDigifizBufferMasked(14, number[(mileage / 10) % 10], 0x7F);
              lx.setDigifizBufferMasked(15, number[(mileage / 1) % 10], 0x70);
              lx.setDigifizBufferMasked(32, number[(mileage / 1) % 10],0xF); 
        }
        else
        {
          if (((mileage / 10) % 10)!=0)
          {
                lx.setDigifizBufferMasked(10, 0, 0x7F);
                lx.setDigifizBufferMasked(11, 0, 0x7F);
                lx.setDigifizBufferMasked(12, 0, 0x7F);
                lx.setDigifizBufferMasked(13, 0, 0x7F);
                lx.setDigifizBufferMasked(14, number[(mileage / 10) % 10], 0x7F);
                lx.setDigifizBufferMasked(15, number[(mileage / 1) % 10], 0x70);
                lx.setDigifizBufferMasked(32, number[(mileage / 1) % 10],0xF);
          }
          else
          {
                lx.setDigifizBufferMasked(10, 0, 0x7F);
                lx.setDigifizBufferMasked(11, 0, 0x7F);
                lx.setDigifizBufferMasked(12, 0, 0x7F);
                lx.setDigifizBufferMasked(13, 0, 0x7F);
                lx.setDigifizBufferMasked(14, 0, 0x7F);
                lx.setDigifizBufferMasked(15, number[(mileage / 1) % 10], 0x70);
                lx.setDigifizBufferMasked(32, number[(mileage / 1) % 10],0xF);
          }
        }
      }
    }
  }
}

void setClockData(uint8_t clock_hours,uint8_t clock_minutes)
{
    uint8_t number_clock[10]={0b11111011,0b01100000,0b10111101,0b11111100,0b01100110,0b11011110,0b11011111,0b11100000,0b11111111,0b11110110};
    if (((clock_hours / 10) % 10)!=0)
    {
        lx.setDigifizBufferMasked(31, number_clock[number_clock[(clock_hours / 10) % 10]],0x7);
        lx.setDigifizBufferMasked(32, number_clock[number_clock[(clock_hours / 10) % 10]],0xF0);
        lx.setDigifizBufferMasked(15, number_clock[(clock_hours / 1) % 10],0x7);
        lx.setDigifizBufferMasked(16, number_clock[(clock_hours / 1) % 10],0xF0);
    }
    else
    {
        lx.setDigifizBufferMasked(31, 0,0x7);
        lx.setDigifizBufferMasked(32, 0,0xF0);
        lx.setDigifizBufferMasked(15, number_clock[(clock_hours / 1) % 10],0x7);
        lx.setDigifizBufferMasked(16, number_clock[(clock_hours / 1) % 10],0xF0);
    }

    //mins
    lx.setDigifizBufferMasked(16, number_clock[(clock_minutes / 10) % 10],0x7);
    lx.setDigifizBufferMasked(17, number_clock[(clock_minutes / 10) % 10],0xF0);

    lx.setDigifizBufferMasked(17, number_clock[(clock_minutes / 1) % 10],0x7);
    lx.setDigifizBufferMasked(18, number_clock[(clock_minutes / 1) % 10],0xF0);
}

void setMFAClockData(uint8_t mfa_clock_hrs,uint8_t mfa_clock_mins)
{
    uint8_t mfa_clock_hours = mfa_clock_hrs;
    uint8_t mfa_clock_minutes = mfa_clock_mins;
    
    uint8_t number_mfa[10]={0b11111011,0b01100000,0b10111101,0b11111100,0b01100110,0b11011110,0b11011111,0b11100000,0b11111111,0b11110110};

    if (mfa_clock_hours>99)
    {
        mfa_clock_hours=99;
        mfa_clock_minutes=99;
    }
    
    if (((mfa_clock_hours / 10) % 10)!=0)
    {
        lx.setDigifizBufferMasked(18, number_mfa[(mfa_clock_hours / 10) % 10],0x7);
        lx.setDigifizBufferMasked(19, number_mfa[(mfa_clock_hours / 10) % 10],0xF0);
        lx.setDigifizBufferMasked(19, number_mfa[(mfa_clock_hours / 1) % 10],0x7);
        lx.setDigifizBufferMasked(20, number_mfa[(mfa_clock_hours / 1) % 10],0xF0);
    }
    else
    {
        lx.setDigifizBufferMasked(18, 0 ,0x7);
        lx.setDigifizBufferMasked(19, 0 ,0xF0);
        lx.setDigifizBufferMasked(19, number_mfa[(mfa_clock_hours / 1) % 10],0x7);
        lx.setDigifizBufferMasked(20, number_mfa[(mfa_clock_hours / 1) % 10],0xF0);
    }

    lx.setDigifizBufferMasked(20, number_mfa[(mfa_clock_minutes / 10) % 10],0x7);
    lx.setDigifizBufferMasked(21, number_mfa[(mfa_clock_minutes / 10) % 10],0xF0);
    lx.setDigifizBufferMasked(21, number_mfa[(mfa_clock_minutes / 1) % 10],0x7);
    lx.setDigifizBufferMasked(22, number_mfa[(mfa_clock_minutes / 1) % 10],0xF0);
}

void setMFADisplayedNumber(int16_t data)
{   
    uint8_t number_mfa[10]={0b11111011,0b01100000,0b10111101,0b11111100,0b01100110,0b11011110,0b11011111,0b11100000,0b11111111,0b11110110};

    
    if (data>=0)
    {
      if (((data / 1000) % 10)!=0)
      {
            lx.setDigifizBufferMasked(18, number_mfa[(data / 1000) % 10],0x7);
            lx.setDigifizBufferMasked(19, number_mfa[(data / 1000) % 10],0xF0);
            lx.setDigifizBufferMasked(19, number_mfa[(data / 100) % 10],0x7);
            lx.setDigifizBufferMasked(20, number_mfa[(data / 100) % 10],0xF0);
      }
      else
      {
        lx.setDigifizBufferMasked(18, 0,0x7);
        lx.setDigifizBufferMasked(19, 0,0xF0);
        lx.setDigifizBufferMasked(19, number_mfa[(data / 100) % 10],0x7);
        lx.setDigifizBufferMasked(20, number_mfa[(data / 100) % 10],0xF0);
      }
      lx.setDigifizBufferMasked(20, number_mfa[(data / 10) % 10],0x7);
      lx.setDigifizBufferMasked(21, number_mfa[(data / 10) % 10],0xF0);
      lx.setDigifizBufferMasked(21, number_mfa[(data / 1) % 10],0x7);
      lx.setDigifizBufferMasked(22, number_mfa[(data / 1) % 10],0xF0);
    }
    else
    {
      //minus values
      uint8_t minus_number=0b100;
      
      if (((data / 1000) % 10)!=0)
      {
            lx.setDigifizBufferMasked(18, number_mfa[(-data / 1000) % 10],0x7);
            lx.setDigifizBufferMasked(19, number_mfa[(-data / 1000) % 10],0xF0);
            lx.setDigifizBufferMasked(19, number_mfa[(-data / 100) % 10],0x7);
            lx.setDigifizBufferMasked(20, number_mfa[(-data / 100) % 10],0xF0);
      }
      else
      {      
        if (((data / 100) % 10)!=0)
        {
            lx.setDigifizBufferMasked(18, minus_number,0x7);
            lx.setDigifizBufferMasked(19, minus_number,0xF0);
            lx.setDigifizBufferMasked(19, number_mfa[(-data / 100) % 10],0x7);
            lx.setDigifizBufferMasked(20, number_mfa[(-data / 100) % 10],0xF0);
        }
        else
        {
            lx.setDigifizBufferMasked(18, 0,0x7);
            lx.setDigifizBufferMasked(19, 0,0xF0);
            lx.setDigifizBufferMasked(19, minus_number,0x7);
            lx.setDigifizBufferMasked(20, minus_number,0xF0);
        }
      }
      lx.setDigifizBufferMasked(20, number_mfa[(-data / 10) % 10],0x7);
      lx.setDigifizBufferMasked(21, number_mfa[(-data / 10) % 10],0xF0);
      lx.setDigifizBufferMasked(21, number_mfa[(-data / 1) % 10],0x7);
      lx.setDigifizBufferMasked(22, number_mfa[(-data / 1) % 10],0xF0);
    }
}

void setFuel(uint8_t litres)
{
    uint8_t number_fuel[10]={0b11111011,0b01100000,0b10111101,0b11111100,0b01100110,0b11011110,0b11011111,0b11100000,0b11111111,0b11110110};
    uint8_t data = litres;
    if (((data / 10) % 10)!=0)
    {
            lx.setDigifizBufferMasked(22, number_fuel[(data / 10) % 10],0x7);
            lx.setDigifizBufferMasked(23, number_fuel[(data / 10) % 10],0xF0);
            lx.setDigifizBufferMasked(23, number_fuel[(data / 1) % 10],0x7);
            lx.setDigifizBufferMasked(24, number_fuel[(data / 1) % 10],0xF0);
    }
    else
    {
            lx.setDigifizBufferMasked(22, 0,0x7);
            lx.setDigifizBufferMasked(23, 0,0xF0);
            lx.setDigifizBufferMasked(23, number_fuel[(data / 1) % 10],0x7);
            lx.setDigifizBufferMasked(24, number_fuel[(data / 1) % 10],0xF0);
    }    
}

void setRPMData(uint16_t data)
{
    uint16_t rpm_segments[9]={0b0, 0b1000,0b1010,0b1110,0b1111,0b11111,0b1011111,0b1111111,0b11111111};
    long long rpm = data;
    rpm*=80;
    rpm/=8000;
    
    if (rpm>80)
      return;
    if (rpm==0)
      rpm=1;
    //RPM = segments count
    uint8_t full_blocks_count = (rpm>>3); // divide by 8
    for (uint8_t i=0;i!=10;i++)
    {
      lx.setDigifizBuffer(9-i, 0);
    }
    for (uint8_t i=0;i!=full_blocks_count;i++)
    {
      lx.setDigifizBuffer(9-i, rpm_segments[8]);
    }
    if (full_blocks_count<10)
    {
    uint8_t part = rpm%8;
    lx.setDigifizBuffer(9-full_blocks_count, rpm_segments[part]);
    }
}

void setSpeedometerData(uint16_t data)
{
    uint8_t number_spd[10]={0b11110011,0b01100000,0b11010110,0b11111100,0b01100101,0b10111101,0b10111111,0b01110000,0b11111111,0b11111101};
    if (((data / 100) % 10)!=0)
    {
          lx.setDigifizBufferMasked(30, number_spd[(data / 100) % 10], 0xFF);
          lx.setDigifizBufferMasked(29, number_spd[(data / 10) % 10], 0xFF);
          lx.setDigifizBufferMasked(28, number_spd[(data / 1) % 10], 0xFF);
    }
    else
    {
        if (((data / 10) % 10)!=0)
        {
          lx.setDigifizBufferMasked(30, 0, 0x7F);
          lx.setDigifizBufferMasked(29, number_spd[(data / 10) % 10], 0xFF);
          lx.setDigifizBufferMasked(28, number_spd[(data / 1) % 10], 0xFF);
        }
        else
        {
          lx.setDigifizBufferMasked(30, 0, 0x7F);
          lx.setDigifizBufferMasked(29, 0, 0xFF);
          lx.setDigifizBufferMasked(28, number_spd[(data / 1) % 10], 0xFF);
        }
    }
}

void fireDigifiz()
{
  lx.fireDigifiz();
}

void setDot(bool value)
{

}

void setFloatDot(bool value)
{
    orig_floatDot = value;
}

void setCoolantData(uint16_t data)
{
    uint16_t coolant_segments[8]={0b0, 0b10000000,0b10100000,0b11100000,0b11100001,0b11100101,0b11100111,0b11101111};
    uint16_t coolant_segments_top[8]={0b0, 0b10000000,0b10100000,0b11100000,0b11110000,0b11110100,0b11110110,0b11111110};
    if (data>24)
      return;
    //RPM = segments count

    uint8_t full_blocks_count1 = 0;
    uint8_t full_blocks_count2 = 0;
    uint8_t full_blocks_count3 = 0;
    if (data<8)
    {
      full_blocks_count1=data%8;
    }
    else
    {
      full_blocks_count1 = 7;  
      if ((data<14))
      {
        full_blocks_count2=(data-7)%8;
      }
      else
      {
        full_blocks_count2 = 7; 
        full_blocks_count3=(data-14)%8;
      }
    }
    lx.setDigifizBuffer(25, coolant_segments[full_blocks_count1]);
    lx.setDigifizBuffer(26, coolant_segments[full_blocks_count2]);
    lx.setDigifizBuffer(27, coolant_segments_top[full_blocks_count3]);
}
#endif
