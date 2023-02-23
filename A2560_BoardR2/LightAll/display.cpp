#include "display.h"

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, 2);
MD_MAX72XX mx2 = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN2, CLK_PIN2, CS_PIN2, 3);


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

void setAll(uint8_t lt)
{
  if (lt)
  {
    for (int i=0;i!=24;i++)
      mx.setColumn(i,0xFF);
    for (int i=0;i!=24;i++)
      mx2.setColumn(i,0xFF);
  }
  else
  {
    for (int i=0;i!=24;i++)
      mx.setColumn(i,0x0);
    for (int i=0;i!=24;i++)
      mx2.setColumn(i,0x0);
  }
}
