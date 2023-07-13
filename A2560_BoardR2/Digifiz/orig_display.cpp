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

#define ORIG_DIGI_CLOCK_PIN A3
#define ORIG_DIGI_STROBE_CLOCK_PIN 19
#define ORIG_DIGI_DATA_PIN 18

#define ORIG_DIGI_CLOCK_PIN_NAME PF3
#define ORIG_DIGI_CLOCK_PORT PORTF

#define ORIG_DIGI_STROBE_CLOCK_PIN_NAME PD2
#define ORIG_DIGI_STROBE_CLOCK_PORT PORTD

#define ORIG_DIGI_DATA_PIN_NAME PD3
#define ORIG_DIGI_DATA_PORT PORTD

uint8_t tr_status = 0x00; // sending, clockbit, 000000
uint16_t data_cnt = 0;
//uint16_t data_payload[33] = {56327, 5, 31744, 79, 24584, 18119, 8199, 17220, 132, 4, 0, 0, 48512, 23947, 59137, 45854, 0, 0, 63488, 65083, 6, 0, 64504, 65007, 6, 0, 0, 0, 0, 0, 0, 0, 8};
const uint8_t rpm_segments[71] = { 64
                            ,65
                            ,66
                            ,67
                            ,68
                            ,69
                            ,70
                            ,71
                            ,72
                            ,73//900
                            ,24
                            ,25
                            ,26
                            ,27
                            ,28
                            ,29
                            ,30
                            ,31//1700
                            ,18
                            ,17
                            ,159//2000
                            ,158
                            ,157
                            ,156
                            ,155
                            ,154
                            ,153
                            ,152
                            ,151//2800
                            ,150
                            ,149
                            ,148
                            ,147
                            ,146
                            ,145//3400
                            ,253
                            ,177//3600
                            ,178
                            ,179
                            ,180
                            ,181
                            ,182
                            ,183
                            ,184
                            ,185
                            ,186
                            ,187
                            ,188
                            ,189
                            ,190
                            ,191//5000
                            ,49
                            ,50
                            ,63
                            ,62
                            ,61
                            ,60
                            ,59
                            ,58
                            ,57
                            ,56 //6000
                            ,105
                            ,104
                            ,103
                            ,102
                            ,101
                            ,100
                            ,99
                            ,98
                            ,97
                            ,96//7000
    };

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
    bool_data_payload[254]=0;
    bool_data_payload[250]=0;
    bool_data_payload[73]=0;
    for (int i=0;i<255;i++)
    {
      bool_data_payload[rpm_segments[i]+3]=0;
    }
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
  bool_data_payload[176+3]=onoff ? 1 : 0; //dur
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
            if (digifiz_parameters.digifiz_options&OPTION_FAHRENHEIT)
            {
              setMFADisplayedNumber((int16_t)getOilTemperatureFahrenheit());
              setFloatDot(false);
            }
            else if (digifiz_parameters.digifiz_options&OPTION_KELVIN)
            {
              setMFADisplayedNumber((int16_t)(getOilTemperature()+273.15f));
              setFloatDot(false);
            }
            else
            {
              setMFADisplayedNumber((int16_t)(getOilTemperature()));
              setFloatDot(true);
            }
            break;
        case MFA_STATE_AIR_TEMP:
            if (digifiz_parameters.digifiz_options&OPTION_FAHRENHEIT)
            {
              setMFADisplayedNumber((int16_t)getAmbientTemperatureFahrenheit());
              setFloatDot(false);
            }
            else if (digifiz_parameters.digifiz_options&OPTION_KELVIN)
            { 
              setMFADisplayedNumber((int16_t)(getAmbientTemperature()+273.15f));
              setFloatDot(false);
            }
            else
            {
              setMFADisplayedNumber((int16_t)getAmbientTemperature());
              setFloatDot(true);
            }
            
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
      bool_data_payload[51+3]=1; //dur
      bool_data_payload[19+3]=0; //km
      bool_data_payload[52+3]=0; //l/100
      bool_data_payload[20+3]=0; //kmh
      bool_data_payload[21+3]=0; //oil
      bool_data_payload[53+3]=0; //air
      break;
    case 1:
      bool_data_payload[51+3]=0; //dur
      bool_data_payload[19+3]=1; //km
      bool_data_payload[52+3]=0; //l/100
      bool_data_payload[20+3]=0; //kmh
      bool_data_payload[21+3]=0; //oil
      bool_data_payload[53+3]=0; //air
      break;
    case 2:
      bool_data_payload[51+3]=0; //dur
      bool_data_payload[19+3]=0; //km
      bool_data_payload[52+3]=1; //l/100
      bool_data_payload[20+3]=0; //kmh
      bool_data_payload[21+3]=0; //oil
      bool_data_payload[53+3]=0; //air
      break;
    case 3:
      bool_data_payload[51+3]=0; //dur
      bool_data_payload[19+3]=0; //km
      bool_data_payload[52+3]=0; //l/100
      bool_data_payload[20+3]=1; //kmh
      bool_data_payload[21+3]=0; //oil
      bool_data_payload[53+3]=0; //air
      break;
    case 4:
      bool_data_payload[51+3]=0; //dur
      bool_data_payload[19+3]=0; //km
      bool_data_payload[52+3]=0; //l/100
      bool_data_payload[20+3]=0; //kmh
      bool_data_payload[21+3]=1; //oil
      bool_data_payload[53+3]=0; //air
      break;
    case 5:
      bool_data_payload[51+3]=0; //dur
      bool_data_payload[19+3]=0; //km
      bool_data_payload[52+3]=0; //l/100
      bool_data_payload[20+3]=0; //kmh
      bool_data_payload[21+3]=0; //oil
      bool_data_payload[53+3]=1; //air
      break;
    case 6:
      bool_data_payload[51+3]=0; //dur
      bool_data_payload[19+3]=0; //km
      bool_data_payload[52+3]=0; //l/100
      bool_data_payload[20+3]=0; //kmh
      bool_data_payload[21+3]=0; //oil
      bool_data_payload[53+3]=0; //air
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
  uint16_t segments1[7] = {75,74,74,74,74,107,106};
  uint16_t segments2[7] = {79,76,77,109,110,111,78};
  uint16_t segments3[7] = {83,80,81,113,114,115,82};
  uint16_t segments4[7] = {87,84,85,117,118,119,86};
  uint16_t segments5[7] = {91,88,89,121,122,123,90};
  uint16_t segments6[7] = {95,92,93,125,126,127,94};
  uint16_t number[10]={0b00111111,0b00100001,0b01011011,0b01110011,0b01100101,0b01110110,0b01111110,0b00100011,0b01111111,0b01110111};
  if (((mileage / 100000) % 10)!=0)
  {
    for (uint8_t i=0;i!=7;i++)
    {
      bool_data_payload[segments1[i]+3] = number[(mileage / 100000) % 10]&(1<<i);
      bool_data_payload[segments2[i]+3] = number[(mileage / 10000) % 10]&(1<<i);
      bool_data_payload[segments3[i]+3] = number[(mileage / 1000) % 10]&(1<<i);
      bool_data_payload[segments4[i]+3] = number[(mileage / 100) % 10]&(1<<i);
      bool_data_payload[segments5[i]+3] = number[(mileage / 10) % 10]&(1<<i);
      bool_data_payload[segments6[i]+3] = number[(mileage / 1) % 10]&(1<<i);
    }
  }
  else
  {
    if (((mileage / 10000) % 10)!=0)
    {
      for (uint8_t i=0;i!=7;i++)
      {
        bool_data_payload[segments1[i]+3] = 0;
        bool_data_payload[segments2[i]+3] = number[(mileage / 10000) % 10]&(1<<i);
        bool_data_payload[segments3[i]+3] = number[(mileage / 1000) % 10]&(1<<i);
        bool_data_payload[segments4[i]+3] = number[(mileage / 100) % 10]&(1<<i);
        bool_data_payload[segments5[i]+3] = number[(mileage / 10) % 10]&(1<<i);
        bool_data_payload[segments6[i]+3] = number[(mileage / 1) % 10]&(1<<i);
      }
    }
    else
    {
      if (((mileage / 1000) % 10)!=0)
      {
        for (uint8_t i=0;i!=7;i++)
        {
          bool_data_payload[segments1[i]+3] = 0;
          bool_data_payload[segments2[i]+3] = 0;
          bool_data_payload[segments3[i]+3] = number[(mileage / 1000) % 10]&(1<<i);
          bool_data_payload[segments4[i]+3] = number[(mileage / 100) % 10]&(1<<i);
          bool_data_payload[segments5[i]+3] = number[(mileage / 10) % 10]&(1<<i);
          bool_data_payload[segments6[i]+3] = number[(mileage / 1) % 10]&(1<<i);
        }
      }
      else
      {
        if (((mileage / 100) % 10)!=0)
        {
          for (uint8_t i=0;i!=7;i++)
          {
            bool_data_payload[segments1[i]+3] = 0;
            bool_data_payload[segments2[i]+3] = 0;
            bool_data_payload[segments3[i]+3] = 0;
            bool_data_payload[segments4[i]+3] = number[(mileage / 100) % 10]&(1<<i);
            bool_data_payload[segments5[i]+3] = number[(mileage / 10) % 10]&(1<<i);
            bool_data_payload[segments6[i]+3] = number[(mileage / 1) % 10]&(1<<i);
          }
        }
        else
        {
          if (((mileage / 10) % 10)!=0)
          {
            for (uint8_t i=0;i!=7;i++)
            {
              bool_data_payload[segments1[i]+3] = 0;
              bool_data_payload[segments2[i]+3] = 0;
              bool_data_payload[segments3[i]+3] = 0;
              bool_data_payload[segments4[i]+3] = 0;
              bool_data_payload[segments5[i]+3] = number[(mileage / 10) % 10]&(1<<i);
              bool_data_payload[segments6[i]+3] = number[(mileage / 1) % 10]&(1<<i);
            }
          }
          else
          {
            for (uint8_t i=0;i!=7;i++)
            {
              bool_data_payload[segments1[i]+3] = 0;
              bool_data_payload[segments2[i]+3] = 0;
              bool_data_payload[segments3[i]+3] = 0;
              bool_data_payload[segments4[i]+3] = 0;
              bool_data_payload[segments5[i]+3] = 0;
              bool_data_payload[segments6[i]+3] = number[(mileage / 1) % 10]&(1<<i);
            }
          }
        }
      }
    }
  }
}

void setClockData(uint8_t clock_hours,uint8_t clock_minutes)
{
    uint16_t segments1[7] = {3,0,1,33,34,35,2};
    uint16_t segments2[7] = {7,4,5,37,38,39,6};
    uint16_t segments3[7] = {11,8,9,41,42,43,10};
    uint16_t segments4[7] = {15,12,13,45,46,47,14};
    uint16_t number[10]={0b00111111,0b00100001,0b01011011,0b01110011,0b01100101,0b01110110,0b01111110,0b00100011,0b01111111,0b01110111};
    if (((clock_hours / 10) % 10)!=0)
    {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = number[(clock_hours / 10) % 10]&(1<<i);
            bool_data_payload[segments2[i]+3] = number[(clock_hours / 1) % 10]&(1<<i);
        }  
    }
    else
    {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = 0;
            bool_data_payload[segments2[i]+3] = number[(clock_hours / 1) % 10]&(1<<i);
        }  
    }

    for (uint8_t i=0;i!=7;i++)
    {
        bool_data_payload[segments3[i]+3] = number[(clock_minutes / 10) % 10]&(1<<i);
        bool_data_payload[segments4[i]+3] = number[(clock_minutes / 1) % 10]&(1<<i);
    } 
}

void setMFAClockData(uint8_t mfa_clock_hrs,uint8_t mfa_clock_mins)
{
    uint8_t mfa_clock_hours = mfa_clock_hrs;
    uint8_t mfa_clock_minutes = mfa_clock_mins;
    uint16_t segments1[7] = {195,192,193,225,226,227,194};
    uint16_t segments2[7] = {199,196,197,229,230,231,198};
    uint16_t segments3[7] = {204,201,202,234,235,236,203};
    uint16_t segments4[7] = {208,205,206,238,239,240,207};
    uint16_t number[10]={0b00111111,0b00100001,0b01011011,0b01110011,0b01100101,0b01110110,0b01111110,0b00100011,0b01111111,0b01110111};

    if (mfa_clock_hours>99)
    {
        mfa_clock_hours=99;
        mfa_clock_minutes=99;
    }
    
    if (((mfa_clock_hours / 10) % 10)!=0)
    {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = number[(mfa_clock_hours / 10) % 10]&(1<<i);
            bool_data_payload[segments2[i]+3] = number[(mfa_clock_hours / 1) % 10]&(1<<i);
        }  
    }
    else
    {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = 0;
            bool_data_payload[segments2[i]+3] = number[(mfa_clock_hours / 1) % 10]&(1<<i);
        }  
    }

    for (uint8_t i=0;i!=7;i++)
    {
        bool_data_payload[segments3[i]+3] = number[(mfa_clock_minutes / 10) % 10]&(1<<i);
        bool_data_payload[segments4[i]+3] = number[(mfa_clock_minutes / 1) % 10]&(1<<i);
    }  
}

void setMFADisplayedNumber(int16_t data)
{   
    uint16_t segments1[7] = {195,192,193,225,226,227,194};
    uint16_t segments2[7] = {199,196,197,229,230,231,198};
    uint16_t segments3[7] = {204,201,202,234,235,236,203};
    uint16_t segments4[7] = {208,205,206,238,239,240,207};
    uint16_t number[10]={0b00111111,0b00100001,0b01011011,0b01110011,0b01100101,0b01110110,0b01111110,0b00100011,0b01111111,0b01110111};

    bool_data_payload[232]= orig_floatDot ? 1 : 0;
    
    if (data>=0)
    {
      if (((data / 1000) % 10)!=0)
      {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = number[(data / 1000) % 10]&(1<<i);
            bool_data_payload[segments2[i]+3] = number[(data / 100) % 10]&(1<<i);
        }  
      }
      else
      {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = 0;
            bool_data_payload[segments2[i]+3] = number[(data / 100) % 10]&(1<<i);
        }  
      }
      for (uint8_t i=0;i!=7;i++)
      {
          bool_data_payload[segments3[i]+3] = number[((data) / 10) % 10]&(1<<i);
          bool_data_payload[segments4[i]+3] = number[((data) / 1) % 10]&(1<<i);
      }  
    }
    else
    {
      //minus values
      uint8_t minus_number = 0b01000000; //minus sign
      
      if (((data / 1000) % 10)!=0)
      {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = number[(-data / 1000) % 10]&(1<<i);
            bool_data_payload[segments2[i]+3] = number[(-data / 100) % 10]&(1<<i);
        }  
      }
      else
      {      
        if (((data / 100) % 10)!=0)
        {
          for (uint8_t i=0;i!=7;i++)
          {
              bool_data_payload[segments1[i]+3] = minus_number&(1<<i);
              bool_data_payload[segments2[i]+3] = number[(-data / 100) % 10]&(1<<i);
          }  
        }
        else
        {
          for (uint8_t i=0;i!=7;i++)
          {
              bool_data_payload[segments1[i]+3] = 0;
              bool_data_payload[segments2[i]+3] = minus_number&(1<<i);
          }  
        }
      }
      for (uint8_t i=0;i!=7;i++)
      {
          bool_data_payload[segments3[i]+3] = number[((-data) / 10) % 10]&(1<<i);
          bool_data_payload[segments4[i]+3] = number[((-data) / 1) % 10]&(1<<i);
      }  
    }
}

void setFuel(uint8_t litres)
{
    uint16_t segments1[7] = {129,222,223,255,160,161,128};
    uint16_t segments2[7] = {133,130,131,163,164,165,132};
    uint16_t number[10]={0b00111111,0b00100001,0b01011011,0b01110011,0b01100101,0b01110110,0b01111110,0b00100011,0b01111111,0b01110111};
    uint8_t data = litres;
    if (((data / 10) % 10)!=0)
    {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = number[(data / 10) % 10]&(1<<i);
            bool_data_payload[segments2[i]+3] = number[(data / 1) % 10]&(1<<i);
        }
    }
    else
    {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = 0;
            bool_data_payload[segments2[i]+3] = number[(data / 1) % 10]&(1<<i);
        }
    }    
}

void setRPMData(uint16_t data)
{
    //return; 
    long long rpm = data;
    rpm*=70;
    rpm/=digifiz_parameters.maxRPM;
    if (rpm>sizeof(rpm_segments))
      return;
    for (int i=rpm;i<70;i++)
    {
      bool_data_payload[rpm_segments[i]+3]=0;
    }
    for (int i=rpm;i>0;i--)
    {
      bool_data_payload[rpm_segments[i]+3]=1;
    }
    bool_data_payload[rpm_segments[0]+3]=1;
}

void setSpeedometerData(uint16_t data)
{
    uint16_t segments1[7] = {252,221,221,221,221,220,221};
    uint16_t segments2[7] = {250,249,248,218,219,216,217};
    uint16_t segments3[7] = {244,245,246,214,215,212,213};
    uint16_t number[10]={0b00111111,0b00100001,0b01011011,0b01110011,0b01100101,0b01110110,0b01111110,0b00100011,0b01111111,0b01110111};
    if (((data / 100) % 10)!=0)
    {
        for (uint8_t i=0;i!=7;i++)
        {
            bool_data_payload[segments1[i]+3] = number[(data / 100) % 10]&(1<<i);
            bool_data_payload[segments2[i]+3] = number[(data / 10) % 10]&(1<<i);
            bool_data_payload[segments3[i]+3] = number[(data / 1) % 10]&(1<<i);
        }
    }
    else
    {
        if (((data / 10) % 10)!=0)
        {
            for (uint8_t i=0;i!=7;i++)
            {
                bool_data_payload[segments1[i]+3] = 0;
                bool_data_payload[segments2[i]+3] = number[(data / 10) % 10]&(1<<i);
                bool_data_payload[segments3[i]+3] = number[(data / 1) % 10]&(1<<i);
            }
        }
        else
        {
            for (uint8_t i=0;i!=7;i++)
            {
                bool_data_payload[segments1[i]+3] = 0;
                bool_data_payload[segments2[i]+3] = 0;
                bool_data_payload[segments3[i]+3] = number[(data / 1) % 10]&(1<<i);
            }
        }
    }
}

void setDot(bool value)
{
  if (!digifiz_parameters.displayDot)
  {
    bool_data_payload[40+3] = 1;
    return;
  }
  bool_data_payload[40+3] = value ? 1 : 0;
  
  if (digifiz_parameters.mfaState==MFA_STATE_TRIP_DURATION)
  {
    bool_data_payload[200+3] = value ? 1 : 0;
    bool_data_payload[232+3] = value ? 1 : 0;
  }
}

void setFloatDot(bool value)
{
    orig_floatDot = value;
}

void setCoolantData(uint16_t data)
{
    const uint16_t coolant_segments[20]={143,142,141,140,139,138,137,136,135,134,166,167,168,169,170,171,172,173,174,175};
    //data is from 0..20
    for (int i=data;i<20;i++)
    {
      bool_data_payload[coolant_segments[i]+3]=0;
    }
    for (int i=data;i>0;i--)
    {
      bool_data_payload[coolant_segments[i]+3]=1;
    }
    
}
#endif
