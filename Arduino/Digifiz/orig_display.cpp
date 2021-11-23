#include "orig_display.h"
#ifdef DIGIFIZ_ORIGINAL_DISPLAY
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

#define ORIG_DIGI_CLOCK_PIN 41
#define ORIG_DIGI_STROBE_CLOCK_PIN 40
#define ORIG_DIGI_DATA_PIN 8

#define ORIG_DIGI_CLOCK_PIN_NAME PG0
#define ORIG_DIGI_CLOCK_PORT PORTG

#define ORIG_DIGI_STROBE_CLOCK_PIN_NAME PG1
#define ORIG_DIGI_STROBE_CLOCK_PORT PORTG

#define ORIG_DIGI_DATA_PIN_NAME PH5
#define ORIG_DIGI_DATA_PORT PORTH

uint8_t tr_status = 0x00; // sending, clockbit, 000000
uint16_t data_cnt = 0;
//uint16_t data_payload[33] = {56327, 5, 31744, 79, 24584, 18119, 8199, 17220, 132, 4, 0, 0, 48512, 23947, 59137, 45854, 0, 0, 63488, 65083, 6, 0, 64504, 65007, 6, 0, 0, 0, 0, 0, 0, 0, 8};
uint8_t bool_data_payload[520] = {
  
1,1,1,

0, 1, 0, 0, 1, 1, 1, 1, //first, second digit clock top 
1, 1, 1, 1, 1, 1, 1, 0, //third, fourth digit clock top 
0, 1, 1, 1, 1, 1, 0, 0, //tacho (1700-1900) + MFA
1, 1, 1, 1, 1, 1, 1, 1, //tacho (1000-1700) 
0, 1, 1, 0, 0, 1, 0, 1, //first and second digit clock bot
0, 1, 1, 1, 1, 0, 1, 1, //third and fourth digit clock bot
1, 1, 1, 0, 1, 0, 0, 0, //tacho (5100-5200) + MFA
1, 1, 1, 1, 1, 1, 1, 1, //tacho (6000-5300)
1, 1, 1, 1, 1, 1, 1, 1, //tacho (0-700)
1, 1, 0, 0, 0, 0, 0, 0, //tacho(800-900)) + mileage(1-2)
1, 1, 1, 1, 0, 1, 1, 1, //mileage(3-4)
1, 1, 0, 1, 0, 1, 0, 0, //mileage(5-6)
1, 1, 1, 1, 1, 1, 1, 1, //tacho (7000-6300)
1, 1, 0, 0, 0, 0, 0, 0, //tacho (6200-6100) + mileage(1-2)
1, 1, 0, 0, 0, 1, 0, 1, //mileage (3-4)  
0, 1, 1, 1, 0, 1, 1, 0, //mileage (5-6)
1, 1, 1, 1, 1, 1, 1, 1, //fuel second digit, coolant T (9-10)
1, 1, 1, 1, 1, 1, 1, 1, //coolant T(1-8)
1, 1, 1, 1, 1, 1, 1, 1, //tacho(3400-2800)
1, 1, 1, 1, 1, 1, 1, 1, //tacho(2700-2000)
1, 1, 1, 1, 1, 1, 1, 1, //fuel down, coolant T(11-12)
1, 1, 1, 1, 1, 1, 1, 1, //coolant T(13-20)
0, 1, 1, 1, 1, 1, 1, 1, //refuel sign, tacho(3600-4200)
1, 1, 1, 1, 1, 1, 1, 1, //tacho(4300-5000)
0, 1, 1, 1, 1, 1, 1, 1, //mfa (1,2 digit top)
1, 0, 0, 0, 1, 1, 1, 1, //mfa (dot, 3,4)
1, 0, 0, 0, 1, 0, 1, 1, //mfa (4), speed (3)
1, 0, 1, 1, 0, 1, 0, 0, //speed(1-2), fuel(1 top)
0, 1, 1, 1, 0, 1, 1, 1, //mfa bottom 1,2
1, 0, 1, 1, 1, 0, 1, 1, //mfa float dot, mfa bot 3,4
1, 0, 0, 0, 1, 1, 1, 1, //mfa bot 4, speed top 3
1, 1, 1, 1, 1, 1, 0, 1, //speed 2-1 top, tacho, fuel shit and etc
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 1, 1, 0, 
1, 1, 1, 0, 0, 0, 1, 1, 
1, 1, 1, 1, 1, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 1, 1, 0, 
1, 1, 1, 1, 1, 1, 1, 1, 
1, 0, 1, 1, 1, 1, 0, 1, 
1, 1, 1, 1, 1, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
1,1,1,1,1};


void initDisplayClockTimer()
{
  // TIMER 1 for interrupt frequency 32786.88524590164 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 32786.88524590164 Hz increments
  OCR1A = 243; // = 16000000 / (1 * 32786.88524590164) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interruptsinterrupts
}


ISR(TIMER1_COMPA_vect){ //~64 kHz
   //interrupt commands for TIMER 0 here
   if ((tr_status&0x40)==0) 
   {
      //transmission for clock
      if (ORIG_DIGI_CLOCK_PORT&(1<<ORIG_DIGI_CLOCK_PIN_NAME)) //-----|_____
      {
        ORIG_DIGI_CLOCK_PORT&=~(1<<ORIG_DIGI_CLOCK_PIN_NAME);
        if ((tr_status&0x10)==0)
          ORIG_DIGI_STROBE_CLOCK_PORT|=(1<<ORIG_DIGI_STROBE_CLOCK_PIN_NAME);
        else
        {
          ORIG_DIGI_DATA_PORT |= (1<<ORIG_DIGI_DATA_PIN_NAME);
          delayMicroseconds(16);
          ORIG_DIGI_DATA_PORT&=~(1<<ORIG_DIGI_DATA_PIN_NAME);
        }
      }
      else //______|-------
      {
        if (((tr_status&0x20)==0)&&((tr_status&0x10)==0))
        {
          ORIG_DIGI_STROBE_CLOCK_PORT|=(1<<ORIG_DIGI_STROBE_CLOCK_PIN_NAME);
        }
        ORIG_DIGI_CLOCK_PORT|=(1<<ORIG_DIGI_CLOCK_PIN_NAME);
        if (tr_status&0x20)
        {
          tr_status&=~0x20;
          tr_status|=0x10;
          delayMicroseconds(8);
          ORIG_DIGI_STROBE_CLOCK_PORT&=~(1<<ORIG_DIGI_STROBE_CLOCK_PIN_NAME);
          delayMicroseconds(2);
          ORIG_DIGI_DATA_PORT&=~(1<<ORIG_DIGI_DATA_PIN_NAME);
        }
        else if (tr_status&0x10)
        {
          tr_status&=~0x10;
          ORIG_DIGI_DATA_PORT |= (1<<ORIG_DIGI_DATA_PIN_NAME);
          delayMicroseconds(8);
          ORIG_DIGI_STROBE_CLOCK_PORT|=(1<<ORIG_DIGI_STROBE_CLOCK_PIN_NAME);
        }
      }
      
      if (tr_status&0x80)
      {
        delayMicroseconds(8);
        if (data_cnt==3)
        {
          ORIG_DIGI_DATA_PORT&=~(1<<ORIG_DIGI_DATA_PIN_NAME);
        }
        if (data_cnt==515)
        {
          ORIG_DIGI_DATA_PORT |= (1<<ORIG_DIGI_DATA_PIN_NAME);
        }
      }
   }
   else
   {
      //middle transition for strobe
      if (tr_status&0x80)
      {
        ORIG_DIGI_STROBE_CLOCK_PORT&=~(1<<ORIG_DIGI_STROBE_CLOCK_PIN_NAME); //16 kHz

        //STROBE_CLOCK is always falling here (STROBE_CLOCK_PIN = 0) at this time
        if (data_cnt>250)
        {
          delayMicroseconds(8);
          if (bool_data_payload[data_cnt])
          {
            ORIG_DIGI_DATA_PORT |= (1<<ORIG_DIGI_DATA_PIN_NAME); 
          }
          else
          {
            ORIG_DIGI_DATA_PORT &= ~(1<<ORIG_DIGI_DATA_PIN_NAME);
          }
        }
        else
        {
            if (data_cnt>3)
            {
              delayMicroseconds(8);
              if (bool_data_payload[data_cnt])
              {
                ORIG_DIGI_DATA_PORT |= (1<<ORIG_DIGI_DATA_PIN_NAME); 
              }
              else
              {
                ORIG_DIGI_DATA_PORT &= ~(1<<ORIG_DIGI_DATA_PIN_NAME);
              }
            }
        }
        data_cnt++;
        if (data_cnt==516)
        {
          data_cnt = 0;
          tr_status &= ~0x80;
          ORIG_DIGI_DATA_PORT |= (1<<ORIG_DIGI_DATA_PIN_NAME);
        }
      }
   }
   tr_status ^= 0x40; //32 kHz
} 

void init_digifiz()
{
  tr_status|=0x20;
}

void initDisplay()
{
    pinMode(MFA1_PIN,OUTPUT);
    pinMode(MFA2_PIN,OUTPUT);
    pinMode(BACKLIGHT_CTL_PIN,OUTPUT);
    pinMode(ORIG_DIGI_CLOCK_PIN,OUTPUT);
    pinMode(ORIG_DIGI_STROBE_CLOCK_PIN,OUTPUT);
    pinMode(ORIG_DIGI_DATA_PIN,OUTPUT);
    digitalWrite(ORIG_DIGI_CLOCK_PIN,HIGH);
    digitalWrite(ORIG_DIGI_STROBE_CLOCK_PIN,HIGH);
    digitalWrite(ORIG_DIGI_DATA_PIN,HIGH);
    initDisplayClockTimer();
    //Clock started
    delay(50);//as in Digifiz, 50 ms delay
    init_digifiz();
    delay(600);
    if (!(tr_status&0x80))
    tr_status|=0x80;
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
  }
  else
  {
      digitalWrite(MFA1_PIN,LOW);
      digitalWrite(MFA2_PIN,HIGH);
  }
}

void setRefuelSign(bool onoff)
{

}

void setCheckEngine(bool onoff)
{
    
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
}

void setBrightness(uint8_t levels)
{
  
}

void setMileage(uint32_t mileage)
{
  
}

void setClockData(uint8_t clock_hours,uint8_t clock_minutes)
{
  
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
}

void setMFADisplayedNumber(int16_t data)
{   
    
}

void setFuel(uint8_t litres)
{
    
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
  
}

void setSpeedometerData(uint16_t data)
{
    uint8_t number[10]={0b01111111,0b01000011,0b10110110,0b11100110,0b11001010,0b11101100,0b11111100,0b01000111,0b11111111,0b11101111};
    if (((data / 100) % 10)!=0)
    {
    
    }
    else
    {
        if (((data / 10) % 10)!=0)
        {
        }
        else
        {
        }
    }
}

void setDot(bool value)
{
  if (!digifiz_parameters.displayDot)
  {
    return;
  }
  if (digifiz_parameters.mfaState==MFA_STATE_TRIP_DURATION)
  {
      
  }
}

void setFloatDot(bool value)
{
    orig_floatDot = value;
}

void setCoolantData(uint16_t data)
{
    uint8_t number[5]={0b0000,0b0001,0b0011,0b0111,0b1111};
    //data is from 0..14
    int blocks_lit = data / 4; //DIG22_0...DIG22_4
    blocks_lit = constrain(blocks_lit,0,3);
    
}
#endif
