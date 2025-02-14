#include "ext_eeprom.h"

ExternalEEPROM myMem; //myMem is external EEPROM (24LC512)
uint8_t external_faulty;
digifiz_pars digifiz_parameters;
EEPROMLoadResult eeprom_load_result = EEPROM_NO_LOAD_ATTEMPT;
uint8_t memory_block_selected = 0;
uint8_t memory_locked = 0;

bool checkMagicBytes()
{    
    uint8_t test1,test2,test3,test4;
    uint8_t cnt = 0;
    for (int j=0;j!=EEPROM_DOUBLING;j++)
    {
      for (cnt=0;cnt!=10;cnt++) //What if we have a wrong negative results???
      {
        //Give it 10 chances
        myMem.get(EXTERNAL_OFFSET+0+EEPROM_GAP_SIZE*j,test1);
        myMem.get(EXTERNAL_OFFSET+1+EEPROM_GAP_SIZE*j,test2);
        myMem.get(EXTERNAL_OFFSET+2+EEPROM_GAP_SIZE*j,test3);
        myMem.get(EXTERNAL_OFFSET+3+EEPROM_GAP_SIZE*j,test4);
        if ((test1=='D')&&
            (test2=='I')&&
            (test3=='G')&&
            (test4=='I'))
        {
            return true;
        }
      }
    }
    return false;
}

bool checkInternalMagicBytes()
{    
    return true;
}


void saveParameters()
{
  #ifdef DISABLE_EEPROM
  return;
  #endif

  // Compute the CRC of the current parameters
  computeCRC();

  // Check if parameters have changed, if so, proceed with writing
  bool dataChanged = false;
  // Buffer to hold the parameters read from EEPROM for comparison
  digifiz_pars buffer;
  
  // Read existing data to compare and check for changes
  EEPROM.get(INTERNAL_OFFSET + 4 + EEPROM_GAP_SIZE * memory_block_selected, buffer);

  // Only proceed if data has changed
  if (memcmp(&buffer, &digifiz_parameters, sizeof(digifiz_parameters)) != 0)
  {
    dataChanged = true;
  }

  // Proceed if data has changed or external EEPROM is not faulty
  if (dataChanged || !external_faulty)
  {
    cli();  // Disable interrupts for the critical section

    // If external EEPROM is available and not faulty, write to it
    if (!external_faulty)
    {
      myMem.put(EXTERNAL_OFFSET + 4 + EEPROM_GAP_SIZE * memory_block_selected, digifiz_parameters);
    }

    // Write to internal EEPROM
    EEPROM.put(INTERNAL_OFFSET + 4 + EEPROM_GAP_SIZE * memory_block_selected, digifiz_parameters);

    sei();  // Re-enable interrupts

    // Optional: Verify the written data
    digifiz_pars verifyBuffer;
    EEPROM.get(INTERNAL_OFFSET + 4 + EEPROM_GAP_SIZE * memory_block_selected, verifyBuffer);

    // If verification fails, log an error or take corrective action
    if (memcmp(&verifyBuffer, &digifiz_parameters, sizeof(digifiz_parameters)) != 0)
    {
      // Handle error
    }

    // Increment memory block pointer
    memory_block_selected++;
    if (memory_block_selected == EEPROM_DOUBLING)
    {
      memory_block_selected = 0;
    }
  }
}
void computeCRC()
{
  uint8_t res = 0;
  uint8_t* digi_buf = (uint8_t*)&digifiz_parameters;
  for (int i=0;i!=CRC_FRAGMENT_SIZE;i++)
  {
    res^=digi_buf[i];
  }
  digifiz_parameters.crc = res;
}

uint8_t getCurrentMemoryBlock()
{
  return memory_block_selected;
}

void load_defaults()
{
    memcpy(digifiz_parameters.preamble,"DIGI",4);
#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
    digifiz_parameters.rpmCoefficient = 3000;
    digifiz_parameters.rpmFilterK = 70;
#else
    digifiz_parameters.rpmCoefficient = 1500;
    digifiz_parameters.rpmFilterK = 70;
#endif
#ifdef DIESEL_MODE
    digifiz_parameters.rpmCoefficient = 400;
    digifiz_parameters.rpmFilterK = 70;
#endif  
    digifiz_parameters.speedCoefficient = 100;
    digifiz_parameters.coolantThermistorB = COOLANT_THERMISTOR_B;
    digifiz_parameters.oilThermistorB = OIL_THERMISTOR_B;
    digifiz_parameters.airThermistorB = AIR_THERMISTOR_B;
    digifiz_parameters.tankMinResistance = 35;
    digifiz_parameters.tankMaxResistance = 265;
    digifiz_parameters.tauCoolant = 2;
    digifiz_parameters.tauOil = 2;
    digifiz_parameters.tauAir = 2;
    digifiz_parameters.tauTank = 2;
    digifiz_parameters.mileage = DEFAULT_MILEAGE*3600L;
    digifiz_parameters.daily_mileage[0] = 0;
    digifiz_parameters.daily_mileage[1] = 0;
    digifiz_parameters.autoBrightness = 1;
    digifiz_parameters.brightnessLevel = 10;
#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
    digifiz_parameters.tankCapacity = 70;
#else
    digifiz_parameters.tankCapacity = 63;//55;
#endif
    digifiz_parameters.mfaState = 0;
    digifiz_parameters.buzzerOff = 1;
#ifdef RPM_8000
#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
    digifiz_parameters.maxRPM = 7000; //Audi digifiz supports only 7000 RPM
#else
    digifiz_parameters.maxRPM = 8000;
#endif
#else
    digifiz_parameters.maxRPM = 7000;
#endif
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
    digifiz_parameters.coolantThermistorDefRes = COOLANT_R_AT_NORMAL_T;
    digifiz_parameters.oilThermistorDefRes = OIL_R_AT_NORMAL_T;
    digifiz_parameters.ambThermistorDefRes = AMBIENT_R_AT_NORMAL_T;
    digifiz_parameters.uptime = 0;
    digifiz_parameters.sign_options.enable_touch_sensor = 1;
    digifiz_parameters.digifiz_options.packed_options = 0;
    
#ifdef DIESEL_MODE
    //digifiz_parameters.maxRPM = 6000;
#endif
#ifdef FUEL_LEVEL_EXPERIMENTAL
    digifiz_parameters.digifiz_options.option_linear_fuel = 0;
#else
    digifiz_parameters.digifiz_options.option_linear_fuel = 1;
#endif
#ifdef MANUFACTURER_MFA_SWITCH
    digifiz_parameters.digifiz_options.mfa_manufacturer = 1;
#endif
    
#ifdef GALLONS
    digifiz_parameters.digifiz_options.option_gallons = 1;
#endif

#ifdef MILES
    digifiz_parameters.digifiz_options.option_miles = 1;
#endif

#ifdef FAHRENHEIT 
    digifiz_parameters.digifiz_options.option_fahrenheit = 1;
#endif

#ifdef KELVIN 
    digifiz_parameters.digifiz_options.option_kelvin = 1;
#endif
    computeCRC();
}

void initEEPROM()
{
    external_faulty = 0;
    load_defaults(); //from table, not from memory
    //Serial.begin(9600);
    //Serial.println("PHL EEPROM test");
    #ifdef DISABLE_EEPROM
    return;
    #endif
    Wire.begin();

    if (myMem.begin() == false) //not found in I2C chain
    {
        //These lines of code execute on most of Digifiz Replica manufactured, 
        //since I do not put external memory on most of them(economical reasons...) 
        external_faulty = true; //mark external as faulty
        //Serial.println("No memory detected. ");
        if (checkInternalMagicBytes()) //check if we have internal memory present.
        {
          eeprom_load_result = EEPROM_CORRUPTED;
          for (int j=0;j!=EEPROM_DOUBLING;j++) //try to read correct fragment "EEPROM_DOUBLING" times
          {
            EEPROM.get(INTERNAL_OFFSET+4+EEPROM_GAP_SIZE*j,digifiz_parameters);
            uint8_t* digi_buf = (uint8_t*)&digifiz_parameters;
            uint8_t crc = 0;
            for(int i=0;i!=CRC_FRAGMENT_SIZE;i++)
            {
              crc^=digi_buf[i];
            }
            if (crc==digifiz_parameters.crc) //fragment is good, stop here. 
            {
              switch (j)
              {
                case 0:
                  eeprom_load_result = EEPROM_OK1;
                  break;
                case 1:
                  eeprom_load_result = EEPROM_OK2;
                  break;
                case 2:
                  eeprom_load_result = EEPROM_OK3;
                  break;
                default:
                  eeprom_load_result = EEPROM_OK_UNKNOWN;
                  break;
              }
              break;
            }  
          }
          if (eeprom_load_result == EEPROM_CORRUPTED)
          {
            load_defaults(); //from table, not from memory
            saveParameters();
            saveParameters();
            saveParameters();
          }
        }
        else
        {
          eeprom_load_result = EEPROM_CORRUPTED;
          //We have corrupted all "EEPROM_DOUBLING" slots in memory, rewrite all the data
          //save it 3 times
          saveParameters();
          saveParameters();
          saveParameters();
        }
    }
    else
    {
        eeprom_load_result = EEPROM_OK_TEST;
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
            saveParameters();
            saveParameters();
            saveParameters();
          }
          else
          { 
            //Magic bytes detected
            //read to digifiz_parameters
            //myMem.get(EXTERNAL_OFFSET+4,digifiz_parameters);
            for (int j=0;j!=EEPROM_DOUBLING;j++)
            {
              myMem.get(INTERNAL_OFFSET+4+EEPROM_GAP_SIZE*j,digifiz_parameters);
              uint8_t* digi_buf = (uint8_t*)&digifiz_parameters;
              uint8_t crc = 0;
              for(int i=0;i!=CRC_FRAGMENT_SIZE;i++)
              {
                crc^=digi_buf[i];
              }
              if (crc==digifiz_parameters.crc)
                break;  
            }
          }
          saveParameters();
          saveParameters();
          saveParameters();
        }
        else
        {
          //Internal memory works
          if (!checkMagicBytes()) //Check external memory first
          {
            //Magic bytes detected in internal EEPROM only
            //write example digifiz parameters to external EEPROM
            for (int j=0;j!=EEPROM_DOUBLING;j++)
            {
              EEPROM.get(INTERNAL_OFFSET+4+EEPROM_GAP_SIZE*j,digifiz_parameters);
              uint8_t* digi_buf = (uint8_t*)&digifiz_parameters;
              uint8_t crc = 0;
              for(int i=0;i!=CRC_FRAGMENT_SIZE;i++)
              {
                crc^=digi_buf[i];
              }
              if (crc==digifiz_parameters.crc)
                break;  
            }
            saveParameters();
            saveParameters();
            saveParameters();
          }
          else
          { 
            //Magic bytes detected in both in internal and external EEPROM(the main case in first DRs manufactured)
            //read to digifiz_parameters
            //write to internal eeprom
            for (int j=0;j!=EEPROM_DOUBLING;j++)
            {
              myMem.get(INTERNAL_OFFSET+4+EEPROM_GAP_SIZE*j,digifiz_parameters);
              uint8_t* digi_buf = (uint8_t*)&digifiz_parameters;
              uint8_t crc = 0;
              for(int i=0;i!=CRC_FRAGMENT_SIZE;i++)
              {
                crc^=digi_buf[i];
              }
              if (crc==digifiz_parameters.crc)
                break;  
            }
            saveParameters();
            saveParameters();
            saveParameters();
          }
        }
   }
   
}


void lockMemory()
{
    memory_locked = 1;
}

void unlockMemory()
{
    memory_locked = 0;
}

EEPROMLoadResult getLoadResult()
{
    return eeprom_load_result;
}
