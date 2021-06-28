#include "display.h"

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, 2);
MD_MAX72XX mx2 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN2, CLK_PIN2, CS_PIN2, 3);

int mRPMData = 4000;

void initDisplay()
{
    mx.begin();
    mx2.begin();
    mx.clear();
    mx2.clear();
}

void setRPM(int rpmdata)
{
    mRPMData=rpmdata;
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
  //PA2 PA3
  /*
  switch(block)
  {
    case MFA_AIR_TEMP:
      break;
    case MFA_OIL_TEMP:
      break;
    case MFA_AVERAGE_SPEED:
      break;
    case MFA_DAILY_MILEAGE:
      break;
    case MFA_DRIVING_TIME:
      break;
    default:
      break;
  }
  */
}

void setMFAType(uint8_t type)
{
  if (type<8)
    mx.setColumn(0, (1<<type));
  else
    mx.setColumn(1, (1<<(type-8)));
}

void setMileage(uint32_t mileage)
{
  uint8_t number[10]={0b01111110,0b01000011,0b10110110,0b11100110,0b11001010,0b11101100,0b11111100,0b01000111,0b11111111,0b11101111};
  mx2.setColumn(0, number[(mileage / 100000) % 10]); 
  mx2.setColumn(1, number[(mileage / 10000) % 10]); 
  mx2.setColumn(2, number[(mileage / 1000) % 10]); 
  mx2.setColumn(3, number[(mileage / 100) % 10]); 
  mx2.setColumn(4, number[(mileage / 10) % 10]); 
  //if (mileage!=0)
    mx2.setColumn(5, number[(mileage / 1) % 10]); 
  //else
  //  mx2.setColumn(5, 0b01111110); 
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
    if (blocks_lit>6) 
      blocks_lit=6; 
    //Serial.println(blocks_lit);
    for (uint8_t col=2; col<2+blocks_lit; col++)
    {
    mx.setColumn(col, 0xff);
    }

    //TODO: this is optional and redundant, consider accurate exclusion for 2+blocks_lit
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

void setDot(bool value)
{
  //mx.setPoint(8, 0, value);
  //mx.setPoint(8, 1, value);
  mx2.setPoint(0,8,value);
  mx2.setPoint(0,9,value);

  mx2.setPoint(0,12,value);
  mx2.setPoint(0,13,value);
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
