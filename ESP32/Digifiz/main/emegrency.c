#include "emergency.h"
#include "reg_inout.h"
#include "esp_log.h"
#include "millis.h"

uint8_t emergency_state = 0;
uint16_t emergencyRPM = 3000;
uint32_t emergencyCounter;
uint8_t last_emergency_state = 0;
void initEmergencyModule()
{
    ESP_LOGI(LOG_TAG, "initEmergencyModule started");
    emergencyCounter = 0;//millis();
    ESP_LOGI(LOG_TAG, "initEmergencyModule started");
}

uint8_t processCHECKEngine()
{
    //TODO implement
    if (digifiz_reg_in.checkEng==1)
    {
        setCheckEngine(false);
    }
    else
    {
        setCheckEngine(true);
    }
    return 0;
}

uint8_t processOilPressure(int mRPM)
{
  //TODO check and implement
    emergency_state = 0;
    if (digifiz_reg_in.oil03==0)
    {
        emergency_state = 1;
    }

#if !defined(EMERGENCY_DISABLE_SENSOR_CHECK) && !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
    //Audi Digifiz also does not have 1.8 bar sensor
    if (((last_emergency_state==0))&&
        (digifiz_reg_in.oil18==1)
        &&(mRPM>emergencyRPM))  //Was a problem with Oil 1.8 bar sensor, and we reached RPM
    {
        emergency_state = 2;
        emergencyCounter = millis();
    }
   
    if (((millis()-emergencyCounter)>1000)&&(last_emergency_state==2)&&
        (digifiz_reg_in.oil18==1)
        &&(mRPM>emergencyRPM))  //It is more than second we are at 2000 RPM and there is no pressure
    {
        //OIL 1.8 error!
        emergency_state = 3;
    }
#endif
    
    last_emergency_state = emergency_state;
    return emergency_state;
}

void checkEmergency(int mRPM)
{
  #ifdef TESTMODE
    //digitalWrite(OIL_LED_PIN,HIGH); //emergency, pressure system
  #else
    emergency_state = processOilPressure(mRPM);
    processCHECKEngine();
    if (emergency_state==0)
    {
      #ifdef USE_DISPLAY_LEDS
        digifiz_reg_out.led_p_oil = 0;
      #endif
      #ifdef DIGIFIZ_LCD_DISPLAY
        setLCDOilIndicator(false);
      #endif
        buzzerOff();
    }
    if (emergency_state==1)
    {
      #ifdef USE_DISPLAY_LEDS
        digifiz_reg_out.led_p_oil = 1; //emergency, but for 0.3 bar sensor only
      #endif
      #ifdef DIGIFIZ_LCD_DISPLAY
        setLCDOilIndicator(true);
      #endif
        buzzerOff();
    }
    if (emergency_state==3)
    {
      #ifdef USE_DISPLAY_LEDS
        digifiz_reg_out.led_p_oil = 1; //emergency, pressure system
      #endif
      #ifdef DIGIFIZ_LCD_DISPLAY
        setLCDOilIndicator(true);
      #endif
        buzzerOn();
    }
  #endif
    
}
