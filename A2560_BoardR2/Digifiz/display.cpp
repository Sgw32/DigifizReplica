#include "display.h"
#ifndef DIGIFIZ_ORIGINAL_DISPLAY
#ifndef DIGIFIZ_LCD_DISPLAY
#ifndef AUDI_DISPLAY
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, 2);
MD_MAX72XX mx2 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN2, CLK_PIN2, CS_PIN2, 3);

extern digifiz_pars digifiz_parameters;

#ifdef EMULATE_RTC
extern RTC_Millis myRTC;
#else
extern RTC_DS3231 myRTC;
#endif

extern DateTime startTime[2];
extern bool clockRunning;
int mRPMData = 4000;
bool floatDot = 0;
extern TimeSpan sinceStart;

bool checkEngineActive;
//int mHour = 0;
//int mMinute = 0;

void initDisplay()
{
    mx.begin();
    mx2.begin();
    mx.clear();
    mx2.clear();
    pinMode(MFA1_PIN,OUTPUT);
    pinMode(MFA2_PIN,OUTPUT);
    pinMode(BACKLIGHT_CTL_PIN,OUTPUT);
}

void setServiceDisplayData(uint8_t data)
{
  
}

void setRPM(int rpmdata)
{
    mRPMData=rpmdata;
}

void setBacklight(bool onoff)
{
    digitalWrite(BACKLIGHT_CTL_PIN, onoff ? HIGH : LOW);
}

void blinking()
{
    // Uses the test function of the MAX72xx to blink the display on and off.
    int  nDelay = 1000;

    PRINTS("\nBlinking");
    mx.clear();

    for (uint8_t col=0; col<12; col++)
    {
    mx.setColumn(col, 0xff);
    delay(100);
    mx.setColumn(col, 0x00);
    delay(100);
    }
}

void blinking2()
{
    // Uses the test function of the MAX72xx to blink the display on and off.
    int  nDelay = 1000;

    PRINTS("\nBlinking");
    mx2.clear();

    for (uint8_t col=0; col<24; col++)
    {
    mx2.setColumn(col, 0xff);
    delay(100);
    mx2.setColumn(col, 0x00);
    delay(100);
    }
}

void setMFABlock(uint8_t block)
{
  if (block&0x1)
  {
      digitalWrite(MFA1_PIN,HIGH);
      digitalWrite(MFA2_PIN,LOW);
  }
  else
  {
      digitalWrite(MFA1_PIN,LOW);
      digitalWrite(MFA2_PIN,HIGH);
  }
}

void setRefuelSign(bool onoff)
{
    mx.setColumn(14, onoff ? 0xff : 0x00);
    mx.setColumn(15, onoff ? 0xff : 0x00);
}

void setCheckEngine(bool onoff)
{
    checkEngineActive = onoff;
    //mx.setPoint(5,1,onoff);
    //mx.setPoint(7,1,onoff);
    //mx.setPoint(0,1,onoff);
}

void displayMFAType(uint8_t mfaType)
{    
    switch(digifiz_parameters.mfaState)
    {
        case MFA_STATE_TRIP_DURATION:
            setMFAClockData(sinceStart.hours(),sinceStart.minutes());
            break;
        case MFA_STATE_TRIP_DISTANCE:
            setMFADisplayedNumber((uint16_t)(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]/3600));
            setFloatDot(false);
            break;
        case MFA_STATE_TRIP_L100KM:
            setMFADisplayedNumber((uint16_t)(digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock]*100));
            //setMFADisplayedNumber((uint16_t)(getCurrentIntakeFuelConsumption()*100.0f));
            setFloatDot(true);
            break;
        case MFA_STATE_TRIP_MEAN_SPEED:
            setMFADisplayedNumber((uint16_t)fabs(digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock]));
            setFloatDot(false);
            break;
        case MFA_STATE_OIL_TEMP:
            #ifdef FAHRENHEIT
              setMFADisplayedNumber((int16_t)getOilTemperatureFahrenheit());
              setFloatDot(false);
            #else
            #ifdef KELVIN  
              setMFADisplayedNumber((int16_t)(getOilTemperature()+273.15f));
              setFloatDot(false);
            #else
              setMFADisplayedNumber((int16_t)(getOilTemperature()));
              setFloatDot(true);
            #endif
            #endif
            break;
        case MFA_STATE_AIR_TEMP:
            #ifdef FAHRENHEIT
              setMFADisplayedNumber((int16_t)getAmbientTemperatureFahrenheit());
              setFloatDot(false);
            #else
            #ifdef KELVIN  
              setMFADisplayedNumber((int16_t)(getAmbientTemperature()+273.15f));
              setFloatDot(false);
            #else
              setMFADisplayedNumber((int16_t)getAmbientTemperature());
              setFloatDot(true);
            #endif
            #endif
            
            break;
        default:
            break;
    }
}

void setMFAType(uint8_t type)
{
  uint8_t checkAdd = (checkEngineActive ? 0b10100001 : 0);
  if (type>6)
  {
      mx.setColumn(0,0);
      mx.setColumn(1,checkAdd);
      return;
  }
  uint8_t mfa1_led[6]={0b00000000,0b00000000,0b00001110,0b00110000,0b11000000,0b00000001};
  uint8_t mfa2_led[6]={0b00001000,0b00000100,0b00000000,0b00000000,0b00000000,0b00000010};
  mx.setColumn(0, mfa1_led[type]);
  mx.setColumn(1, checkAdd|mfa2_led[type]);
}

void setBrightness(uint8_t levels)
{
  #ifndef YELLOW_GREEN_LED
    mx.control(MD_MAX72XX::INTENSITY, constrain(levels,0,0xF));
    mx2.control(MD_MAX72XX::INTENSITY, constrain(levels,0,0xF));
  #else
    mx.control(MD_MAX72XX::INTENSITY, constrain(levels,0,0xF));
    mx2.control(MD_MAX72XX::INTENSITY, constrain(levels,0,0xF));
  #endif
}

void setMileage(uint32_t mileage)
{
  //mileage = 943210;
#ifndef NEW_REVISION
//old
  uint8_t first_number[10]={0b11101110,0b01000011,0b10110110,0b01110110,0b01011010,0b01111100,0b11111100,0b01000111,0b11111111,0b01111111};
#else
//"new"
  uint8_t first_number[10]={0b11101110,0b01000011,0b10110110,0b11010110,0b01011010,0b11011100,0b11111100,0b01000111,0b11111111,0b11011111};
#endif
  uint8_t number[10]=      {0b01111110,0b01000011,0b10110110,0b11100110,0b11001010,0b11101100,0b11111100,0b01000111,0b11111111,0b11101111};

  mx2.setColumn(0, first_number[(mileage / 100000) % 10]); //(mileage / 100000) % 10
  mx2.setColumn(1, number[(mileage / 10000) % 10]); 
  mx2.setColumn(2, number[(mileage / 1000) % 10]); 
  mx2.setColumn(3, number[(mileage / 100) % 10]); 
  mx2.setColumn(4, number[(mileage / 10) % 10]); 
  //if (mileage!=0)
    mx2.setColumn(5, number[(mileage / 1) % 10]); 
  //else
  //  mx2.setColumn(5, 0b01111110); 
}

void setClockData(uint8_t clock_hours,uint8_t clock_minutes)
{
    uint8_t number[10]={0b01111111,0b01000011,0b10110111,0b11100111,0b11001011,0b11101101,0b11111101,0b01000111,0b11111111,0b11101111};
    if (((clock_hours / 10) % 10)!=0)
    {
    mx2.setColumn(8, number[(clock_hours / 10) % 10]); //X-
    mx2.setColumn(9, number[(clock_hours / 1) % 10]); //-X
    }
    else
    {
    mx2.setColumn(8, number[0x00]); //X-
    mx2.setColumn(9, number[(clock_hours / 1) % 10]); //-X
    }

    mx2.setColumn(10, number[(clock_minutes / 10) % 10]); //X-
    mx2.setColumn(11, number[(clock_minutes / 1) % 10]); //-X
}

void setMFAClockData(uint8_t mfa_clock_hrs,uint8_t mfa_clock_mins)
{
    uint8_t mfa_clock_hours = mfa_clock_hrs;
    uint8_t mfa_clock_minutes = mfa_clock_mins;
    if (mfa_clock_hours>99)
    {
        mfa_clock_hours=99;
        mfa_clock_minutes=99;
    }
    uint8_t number[10]={0b01111111,0b01000011,0b10110111,0b11100111,0b11001011,0b11101101,0b11111101,0b01000111,0b11111111,0b11101111};
    if (((mfa_clock_hours / 10) % 10)!=0)
    {
    mx2.setColumn(12, number[(mfa_clock_hours / 10) % 10]); //X-
    mx2.setColumn(13, number[(mfa_clock_hours / 1) % 10]); //-X
    }
    else
    {
    mx2.setColumn(12, number[0x00]); //X-
    mx2.setColumn(13, number[(mfa_clock_hours / 1) % 10]); //-X
    }

    mx2.setColumn(14, number[(mfa_clock_minutes / 10) % 10]); //X-
    mx2.setColumn(15, number[(mfa_clock_minutes / 1) % 10]); //-X
}

void setMFADisplayedNumber(int16_t data)
{   
    if (data>=0)
    {
      uint8_t number[10]={0b01111111,0b01000011,0b10110111,0b11100111,0b11001011,0b11101101,0b11111101,0b01000111,0b11111111,0b11101111};
      uint8_t dotMask = floatDot ? 0b11111110 : 0b11111111;
      if (((data / 1000) % 10)!=0)
      {
      mx2.setColumn(12, dotMask&number[(data / 1000) % 10]); //X-
      mx2.setColumn(13, dotMask&number[(data / 100) % 10]); //-X
      }
      else
      {
      mx2.setColumn(12, dotMask&number[0x00]); //X-
      mx2.setColumn(13, dotMask&number[(data / 100) % 10]); //-X
      }
  
      mx2.setColumn(14, number[(data / 10) % 10]); //X-
      mx2.setColumn(15, number[(data / 1) % 10]); //-X
    }
    else
    {
      //minus values
      uint8_t number[10]={0b01111111,0b01000011,0b10110111,0b11100111,0b11001011,0b11101101,0b11111101,0b01000111,0b11111111,0b11101111};
      uint8_t minus_number = 0b10000001; //minus sign
      
      uint8_t dotMask = floatDot ? 0b11111110 : 0b11111111;
      if (((data / 1000) % 10)!=0)
      {
      mx2.setColumn(12, dotMask&number[((uint8_t)(-data) / 1000) % 10]); //X- //display all number XXXX
      mx2.setColumn(13, dotMask&number[((uint8_t)(-data) / 100) % 10]); //-X 
      }
      else
      {      
        if (((data / 100) % 10)!=0)
        {
          mx2.setColumn(12, dotMask&minus_number); //X- //display minus and number (-XXX)
          mx2.setColumn(13, dotMask&number[((uint8_t)(-data) / 100) % 10]); //-X
        }
        else
        {
          mx2.setColumn(12, dotMask&0x01); //X- //display minus and number (-XX)
          mx2.setColumn(13, dotMask&minus_number); //-X
        }
      }
  
      mx2.setColumn(14, number[((-data) / 10) % 10]); //X-
      mx2.setColumn(15, number[((-data) / 1) % 10]); //-X
    }
}

void setFuel(uint8_t litres)
{
    uint8_t number[10]={0b01111111,0b01000011,0b10110111,0b11100111,0b11001011,0b11101101,0b11111101,0b01000111,0b11111111,0b11101111};
    //uint8_t dotMask = digifiz_parameters.displayDot ? 0b11111111 : 0b11111110;
    if (((litres / 10) % 10)!=0)
    {
    mx2.setColumn(6, number[(litres / 10) % 10]); //X-
    mx2.setColumn(7, number[(litres / 1) % 10]); //-X
    }
    else
    {
    mx2.setColumn(6, number[0x00]); //X-
    mx2.setColumn(7, number[(litres / 1) % 10]); //-X
    }
}

void setRPMData(uint16_t data)
{
    uint8_t number[9]={0b00000000,0b00000010,0b00000110,0b00001110,0b00011110,0b00111110,0b01111110,0b11111110,0b11111111};
    long long leds_lit = data;
    leds_lit*=48;
    leds_lit/=digifiz_parameters.maxRPM;
    //leds_lit=leds_lit;
    int blocks_lit = leds_lit / 8;
    if (blocks_lit>6) 
      blocks_lit=6; 
    //Serial.println(blocks_lit);
    for (uint8_t col=2; col<2+blocks_lit; col++)
    {
    mx.setColumn(col, 0xff);
    }

    //TODO: this is optional and redundant, consider accurate exclusion for 2+blocks_lit
    for (uint8_t col=2+blocks_lit+1; col<8; col++)
    {
    mx.setColumn(col, 0x00);
    }
    mx.setColumn(2+blocks_lit, number[leds_lit%8+1]);
}

void setSpeedometerData(uint16_t data)
{
    uint8_t number[10]={0b01111111,0b01000011,0b10110110,0b11100110,0b11001010,0b11101100,0b11111100,0b01000111,0b11111111,0b11101111};
    if (((data / 100) % 10)!=0)
    {
    mx2.setColumn(16, number[(data / 100) % 10]); //X--
    mx2.setColumn(17, number[(data / 10) % 10]); //-X-
    mx2.setColumn(18, number[(data / 1) % 10]); //--X
    mx2.setColumn(19, number[(data / 100) % 10]); //Y--
    mx2.setColumn(20, number[(data / 10) % 10]); //-Y-
    mx2.setColumn(21, number[(data / 1) % 10]); //--Y
    }
    else
    {
    if (((data / 10) % 10)!=0)
    {
      mx2.setColumn(16, 0x00); //X--
      mx2.setColumn(17, number[(data / 10) % 10]); //-X-
      mx2.setColumn(18, number[(data / 1) % 10]); //--X
      mx2.setColumn(19, 0x00); //Y--
      mx2.setColumn(20, number[(data / 10) % 10]); //-Y-
      mx2.setColumn(21, number[(data / 1) % 10]); //--Y
    }
    else
    {
      mx2.setColumn(16, 0x00); //X--
      mx2.setColumn(17, 0x00); //-X-
      mx2.setColumn(18, number[(data / 1) % 10]); //--X
      mx2.setColumn(19, 0x00); //Y--
      mx2.setColumn(20, 0x00); //-Y-
      mx2.setColumn(21, number[(data / 1) % 10]); //--Y
    }
    }
}

void setDot(bool value)
{
  //mx.setPoint(8, 0, value);
  //mx.setPoint(8, 1, value);
  if (!digifiz_parameters.displayDot)
  {
    mx2.setPoint(0,8,true);
    mx2.setPoint(0,9,true);  
    return;
  }
  mx2.setPoint(0,8,value);
  mx2.setPoint(0,9,value);
  if (digifiz_parameters.mfaState==MFA_STATE_TRIP_DURATION)
  {
    mx2.setPoint(0,12,value);
    mx2.setPoint(0,13,value);
    mx2.setPoint(0,14,value);
  }
}

void setFloatDot(bool value)
{
    floatDot = value;
    mx2.setPoint(0,12,0);
    mx2.setPoint(0,13,0);
    mx2.setPoint(0,14,value);
}

void setCoolantData(uint16_t data)
{
    uint8_t number[5]={0b0000,0b0001,0b0011,0b0111,0b1111};
    //data is from 0..14
    int blocks_lit = data / 4; //DIG22_0...DIG22_4
    blocks_lit = constrain(blocks_lit,0,3);
    for (uint8_t col=8; col<8+blocks_lit; col++)
    {
      mx.setColumn(col, 0xff);
    }
    for (uint8_t col=8+blocks_lit; col<11; col++)
    {
      mx.setColumn(col, 0x00);
    }
    mx.setColumn(8+blocks_lit, number[data%4+1]);
}
#endif
#endif
#endif