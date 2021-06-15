#include <MD_MAX72xx.h>
#include <Wire.h>
#include "MS5611.h"

// Turn on debug statements to the serial output
#define  DEBUG  1

#if  DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTD(x) Serial.println(x, DEC)

#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTD(x)

#endif

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES  2

#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

#define CLK_PIN2   14  // or SCK
#define DATA_PIN2  15  // or MOSI
#define CS_PIN2    17  // or SS


#define  DELAYTIME  100  // in milliseconds


MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, 2);
MD_MAX72XX mx2 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN2, CLK_PIN2, CS_PIN2, 3);

//MS5611 ms5611;
int i=0;
int rpm = 4000;
void blinking()
// Uses the test function of the MAX72xx to blink the display on and off.
{
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
// Uses the test function of the MAX72xx to blink the display on and off.
{
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



void setClockData(uint8_t hours,uint8_t minutes)
{
  uint8_t number[10]={0b01111111,0b01000011,0b10110110,0b11100110,0b11001010,0b11101100,0b11111100,0b01000111,0b11111111,0b11101111};
  if (((hours / 10) % 10)!=0)
  {
    mx2.setColumn(8, number[(hours / 10) % 10]); //X-
    mx2.setColumn(9, number[(hours / 1) % 10]); //-X
  }
  else
  {
    mx2.setColumn(8, number[0x00]); //X-
    mx2.setColumn(9, number[(hours / 1) % 10]); //-X
  }

  mx2.setColumn(10, number[(minutes / 10) % 10]); //X-
  mx2.setColumn(11, number[(minutes / 1) % 10]); //-X
}

void setMFAClockData(uint8_t hours,uint8_t minutes)
{
  uint8_t number[10]={0b01111111,0b01000011,0b10110110,0b11100110,0b11001010,0b11101100,0b11111100,0b01000111,0b11111111,0b11101111};
  if (((hours / 10) % 10)!=0)
  {
    mx2.setColumn(12, number[(hours / 10) % 10]); //X-
    mx2.setColumn(13, number[(hours / 1) % 10]); //-X
  }
  else
  {
    mx2.setColumn(12, number[0x00]); //X-
    mx2.setColumn(13, number[(hours / 1) % 10]); //-X
  }

  mx2.setColumn(14, number[(minutes / 10) % 10]); //X-
  mx2.setColumn(15, number[(minutes / 1) % 10]); //-X
}

void setFuel(uint8_t litres)
{
  uint8_t number[10]={0b01111111,0b01000011,0b10110110,0b11100110,0b11001010,0b11101100,0b11111100,0b01000111,0b11111111,0b11101111};
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
  leds_lit/=7000;
  //leds_lit=leds_lit;
  int blocks_lit = leds_lit / 8;
  Serial.println(blocks_lit);
  for (uint8_t col=2; col<2+blocks_lit; col++)
  {
    mx.setColumn(col, 0xff);
  }
  for (uint8_t col=2+blocks_lit; col<8; col++)
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



void setup() {
  // put your setup code here, to run once:
  //analogReference(INTERNAL);
  /*mlx.begin();  
  mlx2.begin(); 
  if (!ms5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }*/
  
  mx.begin();
  mx2.begin();
  mx.clear();
  mx2.clear();
  #if  DEBUG
  Serial.begin(57600);
  #endif
  PRINTS("\n[MD_MAX72XX Test & Demo]");
}

void loop() {
  // put your main code here, to run repeatedly:
  setClockData(12,34);
  setMFAClockData(23,59);
  setSpeedometerData(i);
  setFuel(i);
  setRPMData(rpm);
  i=i+1;
  rpm=rpm+100;
  if (rpm==7000)
    rpm=0;
  delay(100);
}
