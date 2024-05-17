#include "params.h"
#include "esp_log.h"

const char LOG_TAG[] = "Digifiz Replica Next";

digifiz_pars digifiz_parameters;
uint8_t memory_block_selected = 0;

bool checkMagicBytes()
{    
    uint8_t test1,test2,test3,test4;
    test1 = 0;
    test2 = 0;
    test3 = 0;
    test4 = 0;
    uint8_t cnt = 0;
    for (int j=0;j!=EEPROM_DOUBLING;j++)
    {
      for (cnt=0;cnt!=10;cnt++) 
      {
        //Give it 10 chances
        
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
    uint8_t test1,test2,test3,test4;
    test1 = 0;
    test2 = 0;
    test3 = 0;
    test4 = 0;
    uint8_t cnt = 0;
    for (int j=0;j!=EEPROM_DOUBLING;j++)
    {
      for (cnt=0;cnt!=10;cnt++) //What if we have a wrong negative results???
      {
        //Give it 10 chances
        
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

void saveParameters()
{
  #ifdef DISABLE_EEPROM
  return;
  #endif
  computeCRC();
//   cli();
    // Open NVS namespace
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("digifiz", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Write data to NVS
        nvs_set_blob(my_handle, "s1", &digifiz_parameters, sizeof(digifiz_pars));

        // Commit changes to NVS
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            printf("Error (%s) committing data to NVS!\n", esp_err_to_name(err));
        }

        // Close NVS
        nvs_close(my_handle);
    }
//   sei();
  memory_block_selected++;
  if (memory_block_selected==EEPROM_DOUBLING)
    memory_block_selected=0;
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
    digifiz_parameters.header[0] = 'D';
    digifiz_parameters.header[1] = 'I';
    digifiz_parameters.header[2] = 'G';
    digifiz_parameters.header[3] = 'I';

#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
    digifiz_parameters.rpmCoefficient = 3000;
#else
    digifiz_parameters.rpmCoefficient = 1500;
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
    digifiz_parameters.coolantMin = 60;
    digifiz_parameters.coolantMax = 120;
    digifiz_parameters.medianDispFilterThreshold = 65535; // value below will pass
    digifiz_parameters.coolantThermistorDefRes = 10000;
    digifiz_parameters.uptime = 0;
    digifiz_parameters.digifiz_options = 0;
#ifdef MANUFACTURER_MFA_SWITCH
    digifiz_parameters.digifiz_options |= OPTION_MFA_MANUFACTURER;
#endif
    
#ifdef GALLONS
    digifiz_parameters.digifiz_options |= OPTION_GALLONS;
#endif

#ifdef MILES
    digifiz_parameters.digifiz_options |= OPTION_MILES;
#endif

#ifdef FAHRENHEIT 
    digifiz_parameters.digifiz_options |= OPTION_FAHRENHEIT;
#endif

#ifdef KELVIN 
    digifiz_parameters.digifiz_options |= OPTION_KELVIN;
#endif
    computeCRC();
}


void initEEPROM()
{
    ESP_LOGI(LOG_TAG, "initEEPROM started");
    load_defaults(); //from table, not from memory

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);


    #ifdef DISABLE_EEPROM
    return;
    #endif

    if (checkInternalMagicBytes()) //check if we have internal memory present.
    {
        for (int j=0;j!=EEPROM_DOUBLING;j++) //try to read correct fragment "EEPROM_DOUBLING" times
        {
        uint8_t* digi_buf = (uint8_t*)&digifiz_parameters;
        uint8_t crc = 0;
        for(int i=0;i!=CRC_FRAGMENT_SIZE;i++)
        {
            crc^=digi_buf[i];
        }
        if (crc==digifiz_parameters.crc) //fragment is good, stop here. 
            break;  
        }
    }
    else
    {
        //We have corrupted all "EEPROM_DOUBLING" slots in memory, rewrite all the data
        //save it 4 times
        saveParameters();
        saveParameters();
        saveParameters();
        saveParameters();
    }
    ESP_LOGI(LOG_TAG, "initEEPROM ended");
}