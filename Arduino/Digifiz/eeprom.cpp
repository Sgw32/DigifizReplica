#include "eeprom.h"

ExternalEEPROM myMem;

digifiz_pars digifiz_parameters;

bool checkMagicBytes()
{    
    uint8_t test1,test2,test3,test4;
    myMem.get(0,test1);
    myMem.get(1,test2);
    myMem.get(2,test3);
    myMem.get(3,test4);
    if ((test1=='D')&&
        (test2=='I')&&
        (test3=='G')&&
        (test4=='I'))
    {
        return true;
    }
    return false;
}

void saveParameters()
{
  #ifdef DISABLE_EEPROM
  return;
  #endif
    myMem.put(4,digifiz_parameters);
    //Serial.println("Saved!");
}

void load_defaults()
{
    digifiz_parameters.rpmCoefficient = 3000;
    digifiz_parameters.speedCoefficient = 100;
    digifiz_parameters.coolantThermistorB = 4000;
    digifiz_parameters.oilThermistorB = 4000;
    digifiz_parameters.airThermistorB = 4000;
    digifiz_parameters.tankMinResistance = 35;
    digifiz_parameters.tankMaxResistance = 265;
    digifiz_parameters.tauCoolant = 2;
    digifiz_parameters.tauOil = 2;
    digifiz_parameters.tauAir = 2;
    digifiz_parameters.tauTank = 2;
    digifiz_parameters.mileage = 0;
    digifiz_parameters.daily_mileage[0] = 0;
    digifiz_parameters.daily_mileage[1] = 0;
    digifiz_parameters.autoBrightness = 1;
    digifiz_parameters.brightnessLevel = 10;
    digifiz_parameters.tankCapacity = 60;
    digifiz_parameters.mfaState = 0;
    digifiz_parameters.buzzerOff = 1;
    digifiz_parameters.maxRPM = 7000;
    digifiz_parameters.mfaBlock = 0; //0 or 1
    digifiz_parameters.averageConsumption[0] = 0;
    digifiz_parameters.averageConsumption[1] = 0;
    digifiz_parameters.averageSpeed[0] = 0;
    digifiz_parameters.averageSpeed[1] = 0;
    digifiz_parameters.duration[0] = 0;
    digifiz_parameters.duration[1] = 0;
    digifiz_parameters.displayDot = 0;
    digifiz_parameters.backlight_on = 0; 
}

void initEEPROM()
{
    load_defaults();
    Serial.begin(9600);
    //Serial.println("PHL EEPROM test");
    #ifdef DISABLE_EEPROM
    return;
    #endif
    Wire.begin();

    if (myMem.begin() == false)
    {
        //Serial.println("No memory detected. ");
    }
    else
    {
        //Serial.println("Memory detected!");

        //Serial.print("Mem size in bytes: ");
        //Serial.println(myMem.length());
        if (!checkMagicBytes())
        {
          //Serial.println("No magic bytes detected!");
          //write example digifiz parameters
          myMem.put(0,'D');
          myMem.put(1,'I');
          myMem.put(2,'G');
          myMem.put(3,'I');
          myMem.put(4,digifiz_parameters);
        }
        else
        {
          //Serial.println("Magic bytes detected!");
          //read to digifiz_parameters
          myMem.get(4,digifiz_parameters);
          /*myMem.put(0,'D');
          myMem.put(1,'I');
          myMem.put(2,'G');
          myMem.put(3,'I');
          myMem.put(4,digifiz_parameters);*/
        }
    }
}
