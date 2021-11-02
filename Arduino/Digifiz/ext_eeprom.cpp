#include "ext_eeprom.h"

ExternalEEPROM myMem;
uint8_t external_faulty;
digifiz_pars digifiz_parameters;

bool checkMagicBytes()
{    
    uint8_t test1,test2,test3,test4;
    uint8_t cnt = 0;
    for (cnt=0;cnt!=10;cnt++) //What if we have a wrong negative results???
    {
      //Give it 10 chances
      myMem.get(EXTERNAL_OFFSET+0,test1);
      myMem.get(EXTERNAL_OFFSET+1,test2);
      myMem.get(EXTERNAL_OFFSET+2,test3);
      myMem.get(EXTERNAL_OFFSET+3,test4);
      if ((test1=='D')&&
          (test2=='I')&&
          (test3=='G')&&
          (test4=='I'))
      {
          return true;
      }
    }
    return false;
}

bool checkInternalMagicBytes()
{    
    uint8_t test1,test2,test3,test4;
    uint8_t cnt = 0;
    for (cnt=0;cnt!=10;cnt++) //What if we have a wrong negative results???
    {
      //Give it 10 chances
      EEPROM.get(INTERNAL_OFFSET+0,test1);
      EEPROM.get(INTERNAL_OFFSET+1,test2);
      EEPROM.get(INTERNAL_OFFSET+2,test3);
      EEPROM.get(INTERNAL_OFFSET+3,test4);
      if ((test1=='D')&&
          (test2=='I')&&
          (test3=='G')&&
          (test4=='I'))
      {
          return true;
      }
    }
    return false;
}


void saveParameters()
{
  #ifdef DISABLE_EEPROM
  return;
  #endif
  if (!external_faulty)
  {
    myMem.put(EXTERNAL_OFFSET+4,digifiz_parameters);
  }
  EEPROM.put(INTERNAL_OFFSET+4,digifiz_parameters);
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
    digifiz_parameters.backlight_on = 1; 
    digifiz_parameters.coolantMinResistance = 60;
    digifiz_parameters.coolantMaxResistance = 120;
    digifiz_parameters.medianDispFilterThreshold = 65535; // value below will pass
}

void initEEPROM()
{
    external_faulty = 0;
    load_defaults();
    Serial.begin(9600);
    //Serial.println("PHL EEPROM test");
    #ifdef DISABLE_EEPROM
    return;
    #endif
    Wire.begin();

    if (myMem.begin() == false)
    {
        external_faulty = true;
        //Serial.println("No memory detected. ");
        if (checkInternalMagicBytes())
        {
          EEPROM.get(INTERNAL_OFFSET+4,digifiz_parameters);
        }
        else
        {
          EEPROM.put(INTERNAL_OFFSET+0,'D');
          EEPROM.put(INTERNAL_OFFSET+1,'I');
          EEPROM.put(INTERNAL_OFFSET+2,'G');
          EEPROM.put(INTERNAL_OFFSET+3,'I');
          EEPROM.put(INTERNAL_OFFSET+4,digifiz_parameters);
        }
    }
    else
    {
        //EEPROM doubling mechanism
        //Prefer external over internal
        if (!checkInternalMagicBytes())
        {
          //This situation means it is a first boot/recovered unit
          //Try to recover from EEPROM:
          if (!checkMagicBytes()) //Check external memory first
          {
            //No magic bytes detected both in internal and external 
            //write example digifiz parameters
            myMem.put(EXTERNAL_OFFSET+0,'D');
            myMem.put(EXTERNAL_OFFSET+1,'I');
            myMem.put(EXTERNAL_OFFSET+2,'G');
            myMem.put(EXTERNAL_OFFSET+3,'I');
            myMem.put(EXTERNAL_OFFSET+4,digifiz_parameters);
          }
          else
          { 
            //Magic bytes detected
            //read to digifiz_parameters
            myMem.get(EXTERNAL_OFFSET+4,digifiz_parameters);
          }
          EEPROM.put(INTERNAL_OFFSET+0,'D');
          EEPROM.put(INTERNAL_OFFSET+1,'I');
          EEPROM.put(INTERNAL_OFFSET+2,'G');
          EEPROM.put(INTERNAL_OFFSET+3,'I');
          EEPROM.put(INTERNAL_OFFSET+4,digifiz_parameters);
        }
        else
        {
          //Internal memory works
          if (!checkMagicBytes()) //Check external memory first
          {
            //Magic bytes detected in internal EEPROM only
            //write example digifiz parameters to external EEPROM
            EEPROM.get(INTERNAL_OFFSET+4,digifiz_parameters);
            myMem.put(EXTERNAL_OFFSET+0,'D');
            myMem.put(EXTERNAL_OFFSET+1,'I');
            myMem.put(EXTERNAL_OFFSET+2,'G');
            myMem.put(EXTERNAL_OFFSET+3,'I');
            myMem.put(EXTERNAL_OFFSET+4,digifiz_parameters);
          }
          else
          { 
            //Magic bytes detected in both in internal and external EEPROM(the main case)
            //read to digifiz_parameters
            //write to internal eeprom
            myMem.get(EXTERNAL_OFFSET+4,digifiz_parameters);
            EEPROM.put(INTERNAL_OFFSET+0,'D');
            EEPROM.put(INTERNAL_OFFSET+1,'I');
            EEPROM.put(INTERNAL_OFFSET+2,'G');
            EEPROM.put(INTERNAL_OFFSET+3,'I');
            EEPROM.put(INTERNAL_OFFSET+4,digifiz_parameters);
          }
        }
   }
}
