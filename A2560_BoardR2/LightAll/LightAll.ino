#include "display.h"

void setup() 
{  
  initDisplay(); //Start MAX7219 display driver
}

void loop() 
{
  setAll(0);
  delay(100);
  setAll(1);
  delay(100);
}
