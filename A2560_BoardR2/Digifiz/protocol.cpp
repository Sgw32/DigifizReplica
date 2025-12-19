#include "setup.h"
#include "protocol.h"
#include "mfa.h"
#include "adc.h"

uint32_t statusTime;

uint8_t protocol_locked = 0;

extern float spd_m_speedometer;
extern float averageRPM;

String curCmd = "";
String curCmdUIOD = "";
String curCmdBT = "";

#ifdef EMULATE_RTC
extern RTC_Millis myRTC;
#else
extern RTC_DS3231 myRTC;
#endif

void initComProtocol()
{
#ifdef USE_BTSERIAL
    BTserial.begin(9600);
    changeBLEName();
#endif
#ifdef USE_UIOD
    UIODserial.begin(9600);
    //UIODserial.setTimeout(100);
#endif
#ifdef LOCK_PROTOCOL
    protocol_locked = 1;
#endif
    statusTime = millis();
}

void changeBLEName()
{
    return;
    #ifdef USE_BTSERIAL
    BTserial.println("AT+NAMEDigifiz_PHL");
    delay(100); //delay 100 ms
    while (BTserial.available() > 0)  //supress "OK" message if it sends
    {
       BTserial.read();
    }
    #endif
}

/*
 * #define PARAMETER_RPMCOEFFICIENT 0
#define PARAMETER_SPEEDCOEEFICIENT 1
#define PARAMETER_COOLANTTHERMISTORB 2
#define PARAMETER_OILTHERMISTORB 3
#define PARAMETER_AIRTHERMISTORB 4
#define PARAMETER_TANKMINRESISTANCE 5
#define PARAMETER_TANKMAXRESISTANCE 6
#define PARAMETER_TAU_COOLANT 7
#define PARAMETER_TAU_OIL 8
#define PARAMETER_TAU_AIR 9
#define PARAMETER_TAU_TANK 10
#define PARAMETER_MILEAGE 11
#define PARAMETER_DAILY_MILEAGE 12
#define PARAMETER_AUTO_BRIGHTNESS 13
#define PARAMETER_BRIGHTNESS_LEVEL 14
 * 
 */

extern digifiz_pars digifiz_parameters; 

void processGPIOPinsValue(long value)
{
    #ifdef USE_BTSERIAL
    switch (value)
    {
      case 0:
        BTserial.println("PARAMETER_GET_GPIOA_PINS");
        BTserial.println(PINA);
        break;
      case 1:
        BTserial.println("PARAMETER_GET_GPIOB_PINS");
        BTserial.println(PINB);
        break;
      case 2:
        BTserial.println("PARAMETER_GET_GPIOC_PINS");
        BTserial.println(PINC);
        break;
      case 3:
        BTserial.println("PARAMETER_GET_GPIOD_PINS");
        BTserial.println(PIND);
        break;
      case 4:
        BTserial.println("PARAMETER_GET_GPIOE_PINS");
        BTserial.println(PINE);
        break;
      case 5:
        BTserial.println("PARAMETER_GET_GPIOF_PINS");
        BTserial.println(PINF);
        break;
      case 6:
        BTserial.println("PARAMETER_GET_GPIOG_PINS");
        BTserial.println(PING);
        break;
      case 7:
        BTserial.println("PARAMETER_GET_GPIOH_PINS");
        BTserial.println(PINH);
        break;
      case 8:
        BTserial.println("PARAMETER_GET_GPIOJ_PINS");
        BTserial.println(PINJ);
        break;
      case 9:
        BTserial.println("PARAMETER_GET_GPIOK_PINS");
        BTserial.println(PINK);
        break;
      case 10:
        BTserial.println("PARAMETER_GET_GPIOL_PINS");
        BTserial.println(PINL);
        break;
      default:
        break;
    }
    
    #endif
    
    #ifdef USE_UIOD
    UIODserial.println();
    switch (value)
    {
      case 0:
        UIODserial.println(PINA);
        break;
      case 1:
        UIODserial.println(PINB);
        break;
      case 2:
        UIODserial.println(PINC);
        break;
      case 3:
        UIODserial.println(PIND);
        break;
      case 4:
        UIODserial.println(PINE);
        break;
      case 5:
        UIODserial.println(PINF);
        break;
      case 6:
        UIODserial.println(PING);
        break;
      case 7:
        UIODserial.println(PINH);
        break;
      case 8:
        UIODserial.println(PINJ);
        break;
      case 9:
        UIODserial.println(PINK);
        break;
      case 10:
        UIODserial.println(PINL);
        break;
      default:
        break;
    } 
    #endif
}

void processData(int parameter,long value)
{
  int par = parameter;
  //emergency reset(if requested)
  if (par == PARAMETER_RESET_DIGIFIZ)
  {
            load_defaults();
            saveParameters();
            return;
  }

  if ((protocol_locked)&&(par!=PARAMETER_MEMORY_UNLOCK))
     return;
  
  if (par<PARAMETER_READ_ADDITION)
  {
    switch(par)
    {
      case PARAMETER_RPMCOEFFICIENT:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_RPMCOEFFICIENT");
        #endif
        digifiz_parameters.rpmCoefficient.value = value;
        break;
      case PARAMETER_RPM_QUADRATIC_COEFFICIENT:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_RPM_QUADRATIC_COEFFICIENT");
        #endif
        //digifiz_parameters.rpmQuadraticCoefficient.value = value;
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SPEEDCOEEFICIENT");
        #endif
        digifiz_parameters.speedCoefficient.value = value;
        break;
      case PARAMETER_COOLANTTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANTTHERMISTORB");
        updateADCSettings();
        #endif
        digifiz_parameters.coolantThermistorB.value = value;
        break;  
      case PARAMETER_OILTHERMISTORB:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_OILTHERMISTORB");
        updateADCSettings();
        #endif
        digifiz_parameters.oilThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_AIRTHERMISTORB:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AIRTHERMISTORB");
        #endif
        digifiz_parameters.airThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMINRESISTANCE");
        #endif
        digifiz_parameters.tankMinResistance.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMAXRESISTANCE");
        #endif
        digifiz_parameters.tankMaxResistance.value = value;
        updateADCSettings();
        break; 
      case PARAMETER_TAU_COOLANT:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_COOLANT");
        #endif
        digifiz_parameters.tauCoolant.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_OIL:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_OIL");
        #endif
        digifiz_parameters.tauOil.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_AIR:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_AIR");
        #endif
        digifiz_parameters.tauAir.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_TANK:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_TANK");
        #endif
        digifiz_parameters.tauTank.value = value;
        break;
      case PARAMETER_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MILEAGE");
        #endif
        digifiz_parameters.mileage.value = (uint32_t)value*3600;
        break;
      case PARAMETER_DAILY_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_DAILY_MILEAGE");
        #endif
        if (digifiz_parameters.mfaBlock.value)
        {
          digifiz_parameters.daily_mileage_1.value = value;
        }
        else
        {
          digifiz_parameters.daily_mileage_0.value = value;
        }
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AUTO_BRIGHTNESS");
        #endif
        digifiz_parameters.autoBrightness.value = value;
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BRIGHTNESS_LEVEL");
        #endif
        digifiz_parameters.brightnessLevel.value = value;
        break;
      case PARAMETER_TANK_CAPACITY:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANK_CAPACITY");
        #endif
        digifiz_parameters.tankCapacity.value = value;
        updateADCSettings();
        break;
      case PARAMETER_MFA_STATE:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MFA_STATE");
        #endif
        digifiz_parameters.mfaState.value = value;
        break;
      case PARAMETER_BUZZER_OFF:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BUZZER_OFF");
        #endif
        digifiz_parameters.buzzerOff.value = value;
        break;
      case PARAMETER_MAX_RPM:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_RPM");
        #endif
        digifiz_parameters.maxRPM.value = value;
        break;
      case PARAMETER_NORMAL_RESISTANCE_COOLANT:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_NORMAL_RESISTANCE_COOLANT");
        #endif
        digifiz_parameters.coolantThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_NORMAL_RESISTANCE_OIL:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_NORMAL_RESISTANCE_OIL");
        #endif
        digifiz_parameters.oilThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_NORMAL_RESISTANCE_AMB:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_NORMAL_RESISTANCE_AMB");
        #endif
        digifiz_parameters.ambThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_DOT_OFF:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_DOT_OFF");
        #endif
        digifiz_parameters.displayDot.value = value;
        break;
      case PARAMETER_BACKLIGHT_ON:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BACKLIGHT_ON");
        #endif
        digifiz_parameters.backlight_on.value = value;
        break;
      case PARAMETER_M_D_FILTER:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_M_D_FILTER");
        #endif
        //digifiz_parameters.medianDispFilterThreshold.value = value;
        break;
      case PARAMETER_COOLANT_MAX_R:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANT_MAX_R");
        #endif
        digifiz_parameters.coolantMaxResistance.value = value;
        updateADCSettings();
        break;
      case PARAMETER_COOLANT_MIN_R:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANT_MIN_R");
        #endif
        digifiz_parameters.coolantMinResistance.value = value;
        updateADCSettings();
        break;
      case PARAMETER_COMMAND_MFA_RESET:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COMMAND_MFA_RESET");
        #endif
	      pressMFAReset();
        break;
      case PARAMETER_COMMAND_MFA_MODE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COMMAND_MFA_MODE");
        #endif
        pressMFAMode();
        break;
      case PARAMETER_COMMAND_MFA_BLOCK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COMMAND_MFA_BLOCK");
        #endif
        pressMFABlock();
        break;
      case PARAMETER_RPM_FILTER:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_RPM_FILTER");
        #endif
        digifiz_parameters.rpmFilterK.value = value;
        break;
      case PARAMETER_SET_FUEL_CALC_FUNCTION:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_FUEL_CALC_FUNCTION");
        #endif
        digifiz_parameters.option_linear_fuel.value = value&1;
        break;
      case PARAMETER_SET_RPM_OPTIONS:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_RPM_OPTIONS");
        #endif
        digifiz_parameters.rpmOptions_redline_segments.value = value & 0x1F;
        break;
      case PARAMETER_SET_TEMP_OPTIONS:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_TEMP_OPTIONS");
        #endif
        digifiz_parameters.tempOptions_red_segments.value = value & 0x3;
        digifiz_parameters.tempOptions_sensor_connected_ind.value = (value >> 2) & 0x1;
        digifiz_parameters.tempOptions_alarm_function.value = (value >> 3) & 0x1;
        break;
      case PARAMETER_SET_SIGNAL_OPTIONS:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_SIGNAL_OPTIONS");
        #endif
        digifiz_parameters.signalOptions_use_blink_alt_in.value = value & 0x1;
        digifiz_parameters.signalOptions_enable_touch_sensor.value = (value >> 1) & 0x1;
        //digifiz_parameters.signalOptions_invert_light_input.value = (value >> 2) & 0x1;
        break;
      case PARAMETER_MAX_RPM_THRESHOLD:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_RPM_THRESHOLD");
        #endif
        digifiz_parameters.rpmMaxThreshold.value = value;
        break;
      case PARAMETER_MAX_SPEED_THRESHOLD:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_SPEED_THRESHOLD");
        #endif
        digifiz_parameters.speedMaxThreshold.value = value;
        break;
      case PARAMETER_UPTIME:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_UPTIME");
        #endif
        digifiz_parameters.uptime.value = value;
        break;  
      default:
        break;
    }
  }
  else
  {
    DateTime tme = myRTC.now();
    //commands
    if (par==PARAMETER_SET_HOUR)
    {
      #ifdef USE_BTSERIAL
      BTserial.println("PARAMETER_SET_HOUR");
      #endif
      DateTime newTime1 = DateTime(tme.year(), tme.month(), tme.day(), constrain(value,0,24),tme.minute(),0);
      myRTC.adjust(newTime1);
    }
    else if (par==PARAMETER_TOGGLE_MILES)
    {
      #ifdef USE_BTSERIAL
      BTserial.println("PARAMETER_TOGGLE_MILES");
      #endif
      if (!(digifiz_parameters.option_miles.value))
        digifiz_parameters.option_miles.value = 1;
      else
        digifiz_parameters.option_miles.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_GALLONS)
    {
      #ifdef USE_BTSERIAL
      BTserial.println("PARAMETER_TOGGLE_GALLONS");
      #endif
      if (!(digifiz_parameters.option_gallons.value))
        digifiz_parameters.option_gallons.value = 1;
      else
        digifiz_parameters.option_gallons.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_FAHRENHEIT)
    {
      #ifdef USE_BTSERIAL
      BTserial.println("PARAMETER_TOGGLE_FAHRENHEIT");
      #endif
      if (!(digifiz_parameters.option_fahrenheit.value))
        digifiz_parameters.option_fahrenheit.value = 1;
      else
        digifiz_parameters.option_fahrenheit.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_TOUCH_SENSOR)
    {
      #ifdef USE_BTSERIAL
      BTserial.println("PARAMETER_TOGGLE_TOUCH_SENSOR");
      #endif
      if (!(digifiz_parameters.signalOptions_enable_touch_sensor.value))
        digifiz_parameters.signalOptions_enable_touch_sensor.value = 1;
      else
        digifiz_parameters.signalOptions_enable_touch_sensor.value = 0;
    }
    else if (par==PARAMETER_SAVE_PARAMS)
    {
      #ifdef USE_BTSERIAL
      BTserial.println("PARAMETER_SAVE_PARAMS");
      #endif
      saveParameters();
    }
    else
    {
      if (par==PARAMETER_SET_MINUTE)
      {
          #ifdef USE_BTSERIAL
          BTserial.println("PARAMETER_SET_MINUTE");
          #endif
          DateTime newTime2 = DateTime(tme.year(), tme.month(), tme.day(), tme.hour() ,constrain(value,0,60),0);
          myRTC.adjust(newTime2);
      }
      else
      {
        if (par==PARAMETER_RESET_DAILY_MILEAGE)
        {
          #ifdef USE_BTSERIAL
          BTserial.println("PARAMETER_UPTIME");
          #endif
          if (digifiz_parameters.mfaBlock.value)
          {
            digifiz_parameters.daily_mileage_1.value = 0;
          }
          else
          {
            digifiz_parameters.daily_mileage_0.value = 0;
          }
        }
      } 
    }
    

    //reads
    /*

#define PARAMETER_GET_OIL_TEMPERATURE 249
#define PARAMETER_GET_AMBIENT_TEMPERATURE 248
#define PARAMETER_GET_FUEL_IN_TANK 247
#define PARAMETER_GET_SPEED 246
#define PARAMETER_GET_RPM 245
#define PARAMETER_GET_DAY 241
#define PARAMETER_GET_MONTH 240
#define PARAMETER_GET_YEAR 239
#define PARAMETER_GET_HOUR 238
#define PARAMETER_GET_MINUTE 237

    */
    switch(par)
    {
      case PARAMETER_GET_ACCUMULATED_UPTIME:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_ACCUMULATED_UPTIME");
        BTserial.println(digifiz_parameters.uptime.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.uptime.value);
        #endif
        break;
      case PARAMETER_GET_COOLANT_TEMPERATURE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_COOLANT_TEMPERATURE");
        BTserial.println(getCoolantTemperature());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getCoolantTemperature());
        #endif
        break;
      case PARAMETER_GET_AMBIENT_TEMPERATURE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_AMBIENT_TEMPERATURE");
        BTserial.println(getAmbientTemperature());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getAmbientTemperature());
        #endif
        break;
      case PARAMETER_GET_OIL_TEMPERATURE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_OIL_TEMPERATURE");
        BTserial.println(getOilTemperature());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getOilTemperature());
        #endif
        break;
      case PARAMETER_GET_FUEL_IN_TANK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_FUEL_IN_TANK");
        BTserial.println(getLitresInTank());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getLitresInTank());
        #endif
        break;
      case PARAMETER_GET_SPEED:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_SPEED");
        BTserial.println(spd_m_speedometer);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(spd_m_speedometer);
        #endif
        break;
      case PARAMETER_GET_RPM:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_RPM");
        BTserial.println(averageRPM);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(averageRPM);
        #endif
        break;
      case PARAMETER_GET_DAY:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_DAY");
        BTserial.println(myRTC.now().day());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().day());
        #endif
        break;
      case PARAMETER_GET_MONTH:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_MONTH");
        BTserial.println(myRTC.now().month());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().month());
        #endif
        break;
      case PARAMETER_GET_YEAR:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_YEAR");
        BTserial.println(myRTC.now().year());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().year());
        #endif
        break;
      case PARAMETER_GET_HOUR:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_HOUR");
        BTserial.println(myRTC.now().hour());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().hour());
        #endif
        break;
      case PARAMETER_GET_MINUTE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_MINUTE");
        BTserial.println(myRTC.now().minute());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().minute());
        #endif
        break;
      case PARAMETER_GET_GPIO_PINS:
        processGPIOPinsValue(value);
      case PARAMETER_MEMORY_LOCK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MEMORY_LOCK");
        #endif
        #ifdef USE_UIOD
        #endif
        if (value==123)
           protocol_locked = 1;
        break;        
      case PARAMETER_MEMORY_UNLOCK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MEMORY_UNLOCK");
        #endif
        #ifdef USE_UIOD
        #endif
        if (value==123)
           protocol_locked = 0;
        break;        

      default:
        break;
    }

    par-=PARAMETER_READ_ADDITION;
    
    //reads
    switch(par)
    {
      case PARAMETER_RPMCOEFFICIENT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_RPMCOEFFICIENT");
        BTserial.println(digifiz_parameters.rpmCoefficient.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.rpmCoefficient.value);
        #endif
        break;
      case PARAMETER_RPM_QUADRATIC_COEFFICIENT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_RPM_QUADRATIC_COEFFICIENT");
        BTserial.println(0);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(0);
        #endif
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SPEEDCOEEFICIENT");
        BTserial.println(digifiz_parameters.speedCoefficient.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.speedCoefficient.value);
        #endif
        break;
      case PARAMETER_COOLANTTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANTTHERMISTORB");
        BTserial.println(digifiz_parameters.coolantThermistorB.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.coolantThermistorB.value);
        #endif
        break;  
      case PARAMETER_OILTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_OILTHERMISTORB");
        BTserial.println(digifiz_parameters.oilThermistorB.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.oilThermistorB.value);
        #endif
        break;  
      case PARAMETER_AIRTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AIRTHERMISTORB");
        BTserial.println(digifiz_parameters.airThermistorB.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.airThermistorB.value);
        #endif
        break;  
      case PARAMETER_TANKMINRESISTANCE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMINRESISTANCE");
        BTserial.println(digifiz_parameters.tankMinResistance.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tankMinResistance.value);
        #endif
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMAXRESISTANCE");
        BTserial.println(digifiz_parameters.tankMaxResistance.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tankMaxResistance.value);
        #endif
        break; 
      case PARAMETER_TAU_COOLANT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_COOLANT");
        BTserial.println(digifiz_parameters.tauCoolant.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauCoolant.value);
        #endif
        break;
      case PARAMETER_TAU_OIL:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_OIL");
        BTserial.println(digifiz_parameters.tauOil.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauOil.value);
        #endif
        break;
      case PARAMETER_TAU_AIR:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_AIR");
        BTserial.println(digifiz_parameters.tauAir.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauAir.value);
        #endif
        break;
      case PARAMETER_TAU_TANK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_TANK");
        BTserial.println(digifiz_parameters.tauTank.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauTank.value);
        #endif
        break;
      case PARAMETER_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MILEAGE");
        BTserial.println(digifiz_parameters.mileage.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.mileage.value);
        #endif
        break;
      case PARAMETER_DAILY_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_DAILY_MILEAGE");
        BTserial.println(dailyMileage(digifiz_parameters.mfaBlock.value));
        #endif
        #ifdef USE_UIOD
        UIODserial.println(dailyMileage(digifiz_parameters.mfaBlock.value));
        #endif
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AUTO_BRIGHTNESS");
        BTserial.println(digifiz_parameters.autoBrightness.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.autoBrightness.value);
        #endif
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BRIGHTNESS_LEVEL");
        BTserial.println(digifiz_parameters.brightnessLevel.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.brightnessLevel.value);
        #endif
        break;
      case PARAMETER_TANK_CAPACITY:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANK_CAPACITY");
        BTserial.println(digifiz_parameters.tankCapacity.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tankCapacity.value);
        #endif
        break;
      case PARAMETER_MFA_STATE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MFA_STATE");
        BTserial.println(digifiz_parameters.mfaState.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.mfaState.value);
        #endif
        break;
      case PARAMETER_BUZZER_OFF:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BUZZER_OFF");
        BTserial.println(digifiz_parameters.buzzerOff.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.buzzerOff.value);
        #endif
        break;
      case PARAMETER_MAX_RPM:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_RPM");
        BTserial.println(digifiz_parameters.maxRPM.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.maxRPM.value);
        #endif
        break;
      case PARAMETER_BACKLIGHT_ON:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BACKLIGHT_ON");
        BTserial.println(digifiz_parameters.backlight_on.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.backlight_on.value);
        #endif
        break;
      case PARAMETER_M_D_FILTER:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_M_D_FILTER");
        BTserial.println(65535);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(65535);
        #endif
        break;
      case PARAMETER_COOLANT_MAX_R:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANT_MAX_R");
        BTserial.println(digifiz_parameters.coolantMaxResistance.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.coolantMaxResistance.value);
        #endif
        break;
      case PARAMETER_COOLANT_MIN_R:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANT_MIN_R");
        BTserial.println(digifiz_parameters.coolantMinResistance.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.coolantMinResistance.value);
        #endif
        break;
      case PARAMETER_COMMAND_MFA_RESET:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COMMAND_MFA_RESET");
        #endif
        pressMFAReset();
        break;
      case PARAMETER_COMMAND_MFA_MODE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COMMAND_MFA_MODE");
        #endif
        pressMFAMode();
        break;
      case PARAMETER_COMMAND_MFA_BLOCK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COMMAND_MFA_BLOCK");
        #endif
        pressMFABlock();
        break;
      case PARAMETER_RPM_FILTER:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_RPM_FILTER");
        BTserial.println(digifiz_parameters.rpmFilterK.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.rpmFilterK.value);
        #endif
        break;
      case PARAMETER_SET_FUEL_CALC_FUNCTION:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_FUEL_CALC_FUNCTION");
        BTserial.println(digifiz_parameters.option_linear_fuel.value );
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.option_linear_fuel.value);
        #endif
        break;
      case PARAMETER_SET_RPM_OPTIONS:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_RPM_OPTIONS");
        BTserial.println(digifiz_parameters.rpmOptions_redline_segments.value & 0x1F);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.rpmOptions_redline_segments.value & 0x1F);
        #endif
        break;
      case PARAMETER_SET_TEMP_OPTIONS:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_TEMP_OPTIONS");
        BTserial.println((digifiz_parameters.tempOptions_red_segments.value & 0x3) | ((digifiz_parameters.tempOptions_sensor_connected_ind.value & 0x1)<<2) | ((digifiz_parameters.tempOptions_alarm_function.value &0x1)<<3));
        #endif
        #ifdef USE_UIOD
        UIODserial.println((digifiz_parameters.tempOptions_red_segments.value & 0x3) | ((digifiz_parameters.tempOptions_sensor_connected_ind.value & 0x1)<<2) | ((digifiz_parameters.tempOptions_alarm_function.value &0x1)<<3));
        #endif
        break;
      case PARAMETER_SET_SIGNAL_OPTIONS:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SET_SIGNAL_OPTIONS");
        BTserial.println((digifiz_parameters.signalOptions_use_blink_alt_in.value & 0x1) | ((digifiz_parameters.signalOptions_enable_touch_sensor.value & 0x1)<<1));
        #endif
        #ifdef USE_UIOD
        UIODserial.println((digifiz_parameters.signalOptions_use_blink_alt_in.value & 0x1) | ((digifiz_parameters.signalOptions_enable_touch_sensor.value & 0x1)<<1) | ((digifiz_parameters.signalOptions_invert_light_input.value & 0x1)<<2));
        #endif
        break;
      case PARAMETER_MAX_RPM_THRESHOLD:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_RPM_THRESHOLD");
        BTserial.println(digifiz_parameters.rpmMaxThreshold.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.rpmMaxThreshold.value);
        #endif
        break;
      case PARAMETER_MAX_SPEED_THRESHOLD:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_SPEED_THRESHOLD");
        BTserial.println(digifiz_parameters.speedMaxThreshold.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.speedMaxThreshold.value);
        #endif
        break;
      case PARAMETER_UPTIME:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_UPTIME");
        BTserial.println(digifiz_parameters.uptime.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.uptime.value);
        #endif
        break;  
      default:
        break;
    }
  }
  //saveParameters();
}


//#define V1_BT_PROCESSOR
#define V2_BT_PROCESSOR

/*#define OPTION_MFA_MANUFACTURER 1
#define OPTION_MILES 2
#define OPTION_GALLONS 4
#define OPTION_FAHRENHEIT 8
#define OPTION_KELVIN 16
#define OPTION_LBAR 32*/

void printAbout()
{
  #ifdef USE_BTSERIAL
  BTserial.println("Digifiz Replica by PHOL-LABS.");
  BTserial.println("Fedor Zagumennov,");
  BTserial.println("Pavel Myasnikov,");
  BTserial.println("Cherry Fox / Duplux Indicators,");
  BTserial.println("Egor Avramenko.");
  #endif
}

static void printADC()
{
  BTserial.println("ADC:\n");
  BTserial.println((float)getRawCoolantTemperature());
  BTserial.println((float)getRawGasLevel());
  BTserial.println((float)getRawLightLevel());
  BTserial.println((float)getRawAmbientTemperature());
  BTserial.println((float)getRawOilTemperature());
}

void printHelp()
{
  #ifdef USE_BTSERIAL
  BTserial.println("Digifiz Replica by PHOL-LABS.");
  BTserial.println("Your dashboard is:");
  if (digifiz_parameters.option_mfa_manufacturer.value)
    BTserial.println("MFA ON");
  else
    BTserial.println("MFA OFF");

  if (digifiz_parameters.option_miles.value)
    BTserial.println("MPH");
  else
    BTserial.println("KMH");
  if (digifiz_parameters.option_fahrenheit.value)
    BTserial.println("Fahrenheit");
  else
  {
    if (digifiz_parameters.option_kelvin.value)
      BTserial.println("Kelvin");
    else
      BTserial.println("Celsium");
  }
  if (digifiz_parameters.option_gallons.value)
    BTserial.println("Gallons");
  else
    BTserial.println("Liters");

  BTserial.print(digifiz_parameters.maxRPM.value);
  BTserial.println(" RPM");
  BTserial.print("RPM threshold: ");
  BTserial.println(digifiz_parameters.rpmMaxThreshold.value);
  BTserial.print("Speed threshold: ");
  BTserial.println(digifiz_parameters.speedMaxThreshold.value);
  BTserial.println("BUILD_VERSION");
  #endif
}

extern uint8_t testMode;

void protocolParse()
{
  /*if ((millis()-statusTime)>2000)
  {
    statusTime=millis();
    BTserial.println("Digifiz Status");
  }*/
#ifdef USE_BTSERIAL
#ifdef V1_BT_PROCESSOR
   if (BTserial.available() > 0) 
   {
      // look for the next valid integer in the incoming serial stream:
      int parameter = BTserial.parseInt(); 
      // do it again:
      long value = BTserial.parseInt(); 
      // look for the newline. That's the end of your
      // sentence:
      if (BTserial.read() == '\n') 
      {
        // sends confirmation
        // constrain the values to 0 - 255
        parameter = constrain(parameter, 0, 255);
        //value = constrain(value, 0, 65535);
        if(value<0)
          value=0;
        //BTserial.println(parameter);
        //BTserial.println(value);
        processData(parameter,value);
      }
      while(BTserial.available() > 0) {
        char t = BTserial.read();
      }
   }
#endif
#ifdef V2_BT_PROCESSOR
   if (BTserial.available() > 0) 
   {
      char c_char = BTserial.read();
      if (c_char == '\n')
      {
        //process and wait for new cmd
        String parameter = "";
        String value = "";
        
        if (curCmdBT.indexOf(' ')>0)
        {
          int parameter_p = 0;
          long value_p = 0;
          parameter = curCmdBT.substring(0,curCmdBT.indexOf(' '));
          value = curCmdBT.substring(curCmdBT.indexOf(' '));
          if (parameter=="help")
          {
            printHelp();
          }
          else if (parameter=="about")
          {
            printAbout();
          }
          else if (parameter=="adc")
          {
            printADC();
          }
          else if (parameter=="test_mode")
          {
              digifiz_parameters.option_testmode_on.value=!digifiz_parameters.option_testmode_on.value;
          }
          else if (parameter=="test")
          {
            if (testMode==0)
            {
              testMode = 1;
            }
            else
            {
              testMode = 0;
            }
          }
          else
          {
            parameter_p = constrain(atoi(parameter.c_str()),0,255);
            value_p = atol(value.c_str());
            if(value_p<0)
              value_p=0;
            processData(parameter_p, value_p);
          }
        }
        curCmdBT = "";
      }
      else
      {
        curCmdBT+=c_char;
      }
   }
#endif
#endif
#ifdef USE_UIOD
#ifdef UIOD_PARSE_INPUT
   if (UIODserial.available() > 0) 
   {
      char c_char = UIODserial.read();
      //UIODserial.println(c_char);
      if (c_char == '\n')
      {
        //process and wait for new cmd
        String parameter = "";
        String value = "";
        
        if (curCmdUIOD.indexOf(' ')>0)
        {
          int parameter_p = 0;
          long value_p = 0;
          parameter = curCmdUIOD.substring(0,curCmdUIOD.indexOf(' '));
          value = curCmdUIOD.substring(curCmdUIOD.indexOf(' '));
          parameter_p = constrain(atoi(parameter.c_str()),0,255);
          value_p = atol(value.c_str());
          if(value_p<0)
            value_p=0;
          processData(parameter_p, value_p);
          //UIODserial.print(parameter_p);
          //UIODserial.print(" ");
          //UIODserial.println(value_p);
        }
        curCmdUIOD = "";
      }
      else
      {
        curCmdUIOD+=c_char;
      }
   }
#endif
#endif
}
