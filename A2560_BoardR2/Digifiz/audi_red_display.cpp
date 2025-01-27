#include "audi_red_display.h"
#ifdef AUDI_RED_DISPLAY

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, 4);
STLED316S stled(NBR_OF_DIGIT, STB_CLK_PIN_STLED, CLK_PIN_STLED, DATA_PIN_STLED);
STLED316S stled2(NBR_OF_DIGIT, STB_SPD_PIN_STLED, CLK_PIN_STLED, DATA_PIN_STLED);

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
bool backlightStatus;
uint8_t fuel_ind = 0;
uint8_t backlight1 = 0b11110101;
uint8_t barData_mem = 0;
uint8_t clockHoursAudi = 0;
uint8_t clockMinsAudi = 0;
uint8_t audiOptions = 0b11111101;//0b110;//0x6;

uint16_t clock_h = 0;
uint16_t prev_clock_h = 0;
uint8_t clock_m = 0;

void initDisplay()
{
    mx.begin();
    mx.update(MD_MAX72XX::OFF);
    mx.clear();
    stled.begin(SEG1, SEG2, SEG3, SEG4, SEG5, SEG6, SEG7, SEG8);
    stled.setBrightness(DIGITall, 2);
    stled.setBrightnessLED(LEDall, 1);
    stled.clearDisplay();
    stled.setLED(LEDall, false);
    stled2.begin(SEG1, SEG2, SEG3, SEG4, SEG5, SEG6, SEG7, SEG8);
    stled2.setBrightness(DIGITall, 2);
    stled2.setBrightnessLED(LEDall, 2);
    stled2.clearDisplay();
    stled2.setLED(LEDall, false);

    pinMode(MFA1_PIN,OUTPUT);
    pinMode(MFA2_PIN,OUTPUT);
    pinMode(BACKLIGHT_CTL_PIN,OUTPUT);
    
    REFUEL_SIGN_DDR|=(1<<REFUEL_SIGN_PIN);
    REFUEL_SIGN_PORT&=~(1<<REFUEL_SIGN_PIN);

    CHECK_ENGINE_PORT_DDR|=(1<<CHECK_ENGINE_PIN);
    CHECK_ENGINE_PORT&=~(1<<CHECK_ENGINE_PIN);
    backlightStatus = false;
    checkEngineActive = false;
}

void setMilesKMH(bool onoff)
{
  if (onoff)
  {
    audiOptions = (audiOptions&0b11111100)|0b01;
  }
  else
  {
    audiOptions = (audiOptions&0b11111100)|0b10;
  }
}

void setLBar(bool onoff)
{
  if (onoff)
  {
    //audiOptions = (audiOptions&0b11110011)|0b0100;
  }
  else
  {
      //audiOptions = (audiOptions&0b11110011)|0b1000;
  }
}

void setServiceDisplayData(uint8_t data)
{
  
}

void setAudiOptions(uint8_t options)
{
  audiOptions = options;
}

void setRPM(int rpmdata)
{
    mRPMData=rpmdata;
}

void setBacklight(bool onoff)
{
  backlightStatus = onoff;
  stled.setLED(audiOptions, true);
  stled.setLED(~audiOptions, false);
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

void fireDigifiz()
{
    stled.dispRefreshAll();
    stled2.dispRefreshAll();
    mx.update();
}

void blinking2()
{
    // Uses the test function of the MAX72xx to blink the display on and off.
    int  nDelay = 1000;

    PRINTS("\nBlinking");

    for (uint8_t col=0; col<24; col++)
    {
    
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
  if (!onoff)
    REFUEL_SIGN_PORT&=~(1<<REFUEL_SIGN_PIN);
  else
    REFUEL_SIGN_PORT|=(1<<REFUEL_SIGN_PIN);
}

void setCheckEngine(bool onoff)
{
    checkEngineActive = onoff;
  if (!onoff)
    CHECK_ENGINE_PORT&=~(1<<CHECK_ENGINE_PIN);
  else 
    CHECK_ENGINE_PORT|=(1<<CHECK_ENGINE_PIN);
}

void displayMFAType(uint8_t mfaType)
{    
    switch(digifiz_parameters.mfaState)
    {
        case MFA_STATE_TRIP_MEAN_SPEED:
            setMFADisplayedNumber((uint16_t)fabs(digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock]));
            setFloatDot(true);
            break; 
        case MFA_STATE_TRIP_L100KM:
            setMFADisplayedNumber((uint16_t)(digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock]*100));
            setFloatDot(true);
            break;
        case MFA_STATE_TRIP_FUEL:
            setMFADisplayedNumber((uint16_t)fuel_ind);
            setFloatDot(false);
            break; 
        case MFA_STATE_TRIP_MPG:
            setMFADisplayedNumber((uint16_t)(235.215f/digifiz_parameters.averageConsumption[digifiz_parameters.mfaBlock]));
            setFloatDot(true);
            break;  
        case MFA_STATE_TRIP_MEAN_MPH:
            if (digifiz_parameters.digifiz_options.option_miles)
                setMFADisplayedNumber((uint16_t)fabs(digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock]));
            else
                setMFADisplayedNumber((uint16_t)fabs(digifiz_parameters.averageSpeed[digifiz_parameters.mfaBlock])*0.6214);
            break;
        case MFA_STATE_TRIP_DISTANCE:
            setMFADisplayedNumber((uint16_t)(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]/3600));
            break;
        case MFA_STATE_TRIP_DURATION:
            setMFAClockData(sinceStart.hours(),sinceStart.minutes());
            break;
        default:
            break;
    }
}

void setMFAType(uint8_t type)
{
  //0 kmh
  //1 l100
  //2 fuel
  //3 mpg
  //4 avemph
  //5 range
  //6 elpsdtime
  if (type>6)
  {
      mx.setColumn(0,0);
      mx.setColumn(1,0);
      return;
  }
    uint8_t mfa1_led[7]={0b10,0b10000100,0b1000,0b10000,0b100000,0b1000001,0b00000000};
    uint8_t mfa2_led[7]={0b0,0b0,0b0,0b0,0b0,0b0,0b1111};
  mx.setColumn(0, mfa1_led[type]);
  if (backlightStatus)
  {
    mx.setColumn(1, backlight1|mfa2_led[type]);
    
  }
  else
    mx.setColumn(1, mfa2_led[type]);
}

void setBrightness(uint8_t levels)
{
  mx.control(MD_MAX72XX::INTENSITY, 0xF);
  stled.setBrightness(DIGITall, levels);
  stled.setBrightnessLED(LEDall, 15);
  stled2.setBrightness(DIGITall, levels);
    stled2.setBrightnessLED(LEDall, levels);
}

void setMileage(uint32_t mileage)
{
  //0 ok
  //1 ok
  //2 ok
  //3 ok
  //4 ok
  //5 ok
  //6 ok
  //7 ok
  //8 ok
  //9 ok
  uint8_t number[10]=      {0b01111110,0b00001100,0b10110110,0b10011110,0b11001100,0b11011010,0b11111010,0b00001110,0b11111110,0b11011110};

  mx.setColumn(17, number[(mileage / 100000) % 10]); //(mileage / 100000) % 10
  mx.setColumn(18, number[(mileage / 10000) % 10]); 
  mx.setColumn(19, number[(mileage / 1000) % 10]); 
  mx.setColumn(20, number[(mileage / 100) % 10]); 
  mx.setColumn(21, number[(mileage / 10) % 10]); 
  mx.setColumn(22, number[(mileage / 1) % 10]); 
}

void setDailyMileage(uint32_t mileage)
{
  
}

void setAuxDigit(uint8_t digit)
{
  
}

void setClockData(uint8_t clock_hours,uint8_t clock_minutes)
{
   prev_clock_h = clock_m;
   clock_h = clock_hours;
   clock_m = clock_minutes;
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
    uint8_t dig6 = fuel_ind%10;
    uint8_t dig5 = fuel_ind/10;
    uint8_t dig4 = mfa_clock_hours%10;
    uint8_t dig3 = mfa_clock_hours/10;
    uint8_t dig2 = mfa_clock_minutes%10;
    uint8_t dig1 = mfa_clock_minutes/10;
    //uint8_t number[10]=      {0b01111110,0b00001100,0b10110110,0b10011110,0b11001100,0b11011010,0b11111010,0b00001110,0b11111110,0b11011110};
    uint32_t num=(uint32_t)dig2*1000+(uint32_t)dig1*100+(uint32_t)dig4*10+(uint32_t)dig3+(uint32_t)dig5*10000+(uint32_t)dig6*100000;
    stled.dispDec(num);
}

void setMFADisplayedNumber(int16_t data)
{
    uint8_t mfa_clock_hours = (data/100)%100;
    uint8_t mfa_clock_minutes = (data)%100;
    setMFAClockData(mfa_clock_hours,mfa_clock_minutes);
}

void setFuel(uint8_t litres)
{
    fuel_ind = litres;
    barData_mem = (uint8_t)(((float)fuel_ind/digifiz_parameters.tankCapacity)*17);
}

void setRPMData(uint16_t data)
{
  //return;
    uint8_t number[9]={0b00000000,0b00000010,0b00000110,0b00001110,0b00011110,0b00111110,0b01111110,0b11111110,0b11111111};
    long long leds_lit = constrain(data,1000,7000);
    leds_lit-=1000; 
    leds_lit*=32;
    leds_lit/=6000;
    leds_lit = constrain(leds_lit,2,32);
    int blocks_lit = leds_lit / 8;
    if (blocks_lit>6) 
      blocks_lit=6; 
    for (uint8_t col=2; col<2+blocks_lit; col++)
    {
    mx.setColumn(col, 0xff);
    }
    for (uint8_t col=2+blocks_lit+1; col<6; col++)
    {
    mx.setColumn(col, 0x00);
    }
    mx.setColumn(2+blocks_lit, number[leds_lit%8+1]);

    if (backlightStatus)
    {
      mx.setColumn(6,0xFF);
      mx.setColumn(7,0xFF);
    }
}

void setSpeedometerData(uint16_t data)
{
    stled2.dispUdecRevDual(data,clock_h*100+clock_m);
    if (prev_clock_h!=clock_m)
    {
      stled2.setLEDDigit((uint8_t)(clock_h/10));
      prev_clock_h = clock_m;
    }
}

void setBarData(uint8_t data)
{
  barData_mem = data;
}

void setDot(bool value)
{

}

void setFloatDot(bool value)
{
        
}

void setCoolantData(uint16_t data)
{
    //data = 9;
  
    uint8_t number[5]={0b0000,0b0001,0b0011,0b0111,0b1111};
    uint8_t numberBar[4]={0b0000,0b0100,0b1100,0b11100};
    uint8_t number2[5]={0b0000,0b0010,0b0110,0b1110,0b11110};
    uint8_t number3[5]={0b11110,0b111110,0b1111110,0b1111110,0b11111110};
    //data is from 0..17
    int blocks_lit = data / 4; //DIG22_0...DIG22_4
    blocks_lit = constrain(blocks_lit,0,4);


    int blocks_lit2 = barData_mem / 4; //DIG22_0...DIG22_4
    blocks_lit2 = constrain(blocks_lit2,0,3);

    
    for (uint8_t col=8; col<8+blocks_lit; col++)
    {
      mx.setColumn(col, 0xff);
    }
    mx.setColumn(8+blocks_lit, 0b1100000|number[data%4+1]);
    for (uint8_t col=8+blocks_lit+1; col<12; col++)
    {
      mx.setColumn(col, 0b1100000);
    }

    
    
    if ((8+blocks_lit)!=12)
    {
      
      if (barData_mem<4)
        mx.setColumn(12, numberBar[barData_mem%4]);
      else
        mx.setColumn(12, 0b11100);
    }
    else
    {
      if (barData_mem<4)
        mx.setColumn(12, number[data%4+1]|numberBar[barData_mem%4]);
      else
        mx.setColumn(12, number[data%4+1]|0b11100);
    }
    
    for (uint8_t col=13; col<12+blocks_lit2; col++)
    {
      mx.setColumn(col, 0xff);
    }
    for (uint8_t col=13+blocks_lit2; col<16; col++)
    {
      mx.setColumn(col, 0x00);
    }
    if ((barData_mem>3)&&(barData_mem<16))
      mx.setColumn(12+blocks_lit2, number2[barData_mem%4+1]);
    if (barData_mem>15)
      mx.setColumn(15,number3[barData_mem%4+1]);

      //mx.setColumn(8, );
    //mx.setColumn(9, );
      //mx.setColumn(15, 0b1100000);
}
#endif
