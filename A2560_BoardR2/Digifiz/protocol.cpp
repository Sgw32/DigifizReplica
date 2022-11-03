#include "protocol.h"
#include "mfa.h"
#include "adc.h"
#define BTserial Serial
#define UIODserial Serial1

uint32_t statusTime;

extern float spd_m_speedometer;
extern float averageRPM;

#ifdef EMULATE_RTC
extern RTC_Millis myRTC;
#else
extern RTC_DS3231 myRTC;
#endif

#define USE_UIOD

void initComProtocol()
{
    BTserial.begin(9600);
    changeBLEName();
#ifdef USE_UIOD
    UIODserial.begin(115200);
#endif
    statusTime = millis();
}

void changeBLEName()
{
    return;
    BTserial.println("AT+NAMEDigifiz_PHL");
    delay(100); //delay 100 ms
    while (BTserial.available() > 0)  //supress "OK" message if it sends
    {
       BTserial.read();
    }
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
  
  if (par<PARAMETER_READ_ADDITION)
  {
    switch(par)
    {
      case PARAMETER_RPMCOEFFICIENT:
        BTserial.println("PARAMETER_RPMCOEFFICIENT");
        digifiz_parameters.rpmCoefficient = value;
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        BTserial.println("PARAMETER_SPEEDCOEEFICIENT");
        digifiz_parameters.speedCoefficient = value;
        break;
      case PARAMETER_COOLANTTHERMISTORB:
        BTserial.println("PARAMETER_COOLANTTHERMISTORB");
        digifiz_parameters.coolantThermistorB = value;
        break;  
      case PARAMETER_OILTHERMISTORB:
        BTserial.println("PARAMETER_OILTHERMISTORB");
        digifiz_parameters.oilThermistorB = value;
        break;  
      case PARAMETER_AIRTHERMISTORB:
        BTserial.println("PARAMETER_AIRTHERMISTORB");
        digifiz_parameters.airThermistorB = value;
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        BTserial.println("PARAMETER_TANKMINRESISTANCE");
        digifiz_parameters.tankMinResistance = value;
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        BTserial.println("PARAMETER_TANKMAXRESISTANCE");
        digifiz_parameters.tankMaxResistance = value;
        break; 
      case PARAMETER_TAU_COOLANT:
        BTserial.println("PARAMETER_TAU_COOLANT");
        digifiz_parameters.tauCoolant = value;
        break;
      case PARAMETER_TAU_OIL:
        BTserial.println("PARAMETER_TAU_OIL");
        digifiz_parameters.tauOil = value;
        break;
      case PARAMETER_TAU_AIR:
        BTserial.println("PARAMETER_TAU_AIR");
        digifiz_parameters.tauAir = value;
        break;
      case PARAMETER_TAU_TANK:
        BTserial.println("PARAMETER_TAU_TANK");
        digifiz_parameters.tauTank = value;
        break;
      case PARAMETER_MILEAGE:
        BTserial.println("PARAMETER_MILEAGE");
        digifiz_parameters.mileage = (uint32_t)value*3600;
        break;
      case PARAMETER_DAILY_MILEAGE:
        BTserial.println("PARAMETER_DAILY_MILEAGE");
        digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock] = value;
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
        BTserial.println("PARAMETER_AUTO_BRIGHTNESS");
        digifiz_parameters.autoBrightness = value;
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
        BTserial.println("PARAMETER_BRIGHTNESS_LEVEL");
        digifiz_parameters.brightnessLevel = value;
        break;
      case PARAMETER_TANK_CAPACITY:
        BTserial.println("PARAMETER_TANK_CAPACITY");
        digifiz_parameters.tankCapacity = value;
        break;
      case PARAMETER_MFA_STATE:
        BTserial.println("PARAMETER_MFA_STATE");
        digifiz_parameters.mfaState = value;
        break;
      case PARAMETER_BUZZER_OFF:
        BTserial.println("PARAMETER_BUZZER_OFF");
        digifiz_parameters.buzzerOff = value;
        break;
      case PARAMETER_MAX_RPM:
        BTserial.println("PARAMETER_MAX_RPM");
        digifiz_parameters.maxRPM = value;
        break;
      case PARAMETER_DOT_OFF:
        BTserial.println("PARAMETER_DOT_OFF");
        digifiz_parameters.displayDot = value;
        break;
      case PARAMETER_BACKLIGHT_ON:
        BTserial.println("PARAMETER_BACKLIGHT_ON");
        digifiz_parameters.backlight_on = value;
        break;
      case PARAMETER_M_D_FILTER:
        BTserial.println("PARAMETER_M_D_FILTER");
        digifiz_parameters.medianDispFilterThreshold = value;
        break;
      case PARAMETER_COOLANT_MAX_R:
        BTserial.println("PARAMETER_COOLANT_MAX_R");
        digifiz_parameters.coolantMaxResistance = value;
        break;
      case PARAMETER_COOLANT_MIN_R:
        BTserial.println("PARAMETER_COOLANT_MIN_R");
        digifiz_parameters.coolantMinResistance = value;
        break;
      case PARAMETER_COMMAND_MFA_RESET:
        BTserial.println("PARAMETER_COMMAND_MFA_RESET");
	pressMFAReset();
        break;
      case PARAMETER_COMMAND_MFA_MODE:
        BTserial.println("PARAMETER_COMMAND_MFA_MODE");
        pressMFAMode();
        break;
      case PARAMETER_COMMAND_MFA_BLOCK:
        BTserial.println("PARAMETER_COMMAND_MFA_BLOCK");
        pressMFABlock();
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
            DateTime newTime1 = DateTime(tme.year(), tme.month(), tme.day(), constrain(value,0,24),tme.minute(),0);
            myRTC.adjust(newTime1);
    }
    else
    {
      if (par==PARAMETER_SET_MINUTE)
      {
          DateTime newTime2 = DateTime(tme.year(), tme.month(), tme.day(), tme.hour() ,constrain(value,0,60),0);
          myRTC.adjust(newTime2);
      }
      else
      {
        if (par==PARAMETER_RESET_DAILY_MILEAGE)
        {
          digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock] = 0;
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
        BTserial.println("PARAMETER_GET_ACCUMULATED_UPTIME");
        BTserial.println(digifiz_parameters.uptime);
        UIODserial.println(digifiz_parameters.uptime);
        break;
      case PARAMETER_GET_COOLANT_TEMPERATURE:
        BTserial.println("PARAMETER_GET_COOLANT_TEMPERATURE");
        BTserial.println(getCoolantTemperature());
        UIODserial.println(getCoolantTemperature());
        break;
      case PARAMETER_GET_FUEL_IN_TANK:
        BTserial.println("PARAMETER_GET_FUEL_IN_TANK");
        BTserial.println(getLitresInTank());
        UIODserial.println(getLitresInTank());
        break;
      case PARAMETER_GET_SPEED:
        BTserial.println("PARAMETER_GET_SPEED");
        BTserial.println(spd_m_speedometer);
        UIODserial.println(spd_m_speedometer);
        break;
      case PARAMETER_GET_RPM:
        BTserial.println("PARAMETER_GET_RPM");
        BTserial.println(averageRPM);
        UIODserial.println(averageRPM);
        break;
      case PARAMETER_GET_DAY:
        BTserial.println("PARAMETER_GET_DAY");
        BTserial.println(myRTC.now().day());
        UIODserial.println(myRTC.now().day());
        break;
      case PARAMETER_GET_MONTH:
        BTserial.println("PARAMETER_GET_MONTH");
        BTserial.println(myRTC.now().month());
        UIODserial.println(myRTC.now().month());
        break;
      case PARAMETER_GET_YEAR:
        BTserial.println("PARAMETER_GET_YEAR");
        BTserial.println(myRTC.now().year());
        UIODserial.println(myRTC.now().year());
        break;
      default:
        break;
    }

    par-=PARAMETER_READ_ADDITION;
    
    //reads
    switch(par)
    {
      case PARAMETER_RPMCOEFFICIENT:
        BTserial.println("PARAMETER_RPMCOEFFICIENT");
        BTserial.println(digifiz_parameters.rpmCoefficient);
        UIODserial.println(digifiz_parameters.rpmCoefficient);
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        BTserial.println("PARAMETER_SPEEDCOEEFICIENT");
        BTserial.println(digifiz_parameters.speedCoefficient);
        UIODserial.println(digifiz_parameters.speedCoefficient);
        break;
      case PARAMETER_COOLANTTHERMISTORB:
        BTserial.println("PARAMETER_COOLANTTHERMISTORB");
        BTserial.println(digifiz_parameters.coolantThermistorB);
        UIODserial.println(digifiz_parameters.coolantThermistorB);
        break;  
      case PARAMETER_OILTHERMISTORB:
        BTserial.println("PARAMETER_OILTHERMISTORB");
        BTserial.println(digifiz_parameters.oilThermistorB);
        UIODserial.println(digifiz_parameters.oilThermistorB);
        break;  
      case PARAMETER_AIRTHERMISTORB:
        BTserial.println("PARAMETER_AIRTHERMISTORB");
        BTserial.println(digifiz_parameters.airThermistorB);
        UIODserial.println(digifiz_parameters.airThermistorB);
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        BTserial.println("PARAMETER_TANKMINRESISTANCE");
        BTserial.println(digifiz_parameters.tankMinResistance);
        UIODserial.println(digifiz_parameters.tankMinResistance);
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        BTserial.println("PARAMETER_TANKMAXRESISTANCE");
        BTserial.println(digifiz_parameters.tankMaxResistance);
        UIODserial.println(digifiz_parameters.tankMaxResistance);
        break; 
      case PARAMETER_TAU_COOLANT:
        BTserial.println("PARAMETER_TAU_COOLANT");
        BTserial.println(digifiz_parameters.tauCoolant);
        UIODserial.println(digifiz_parameters.tauCoolant);
        break;
      case PARAMETER_TAU_OIL:
        BTserial.println("PARAMETER_TAU_OIL");
        BTserial.println(digifiz_parameters.tauOil);
        UIODserial.println(digifiz_parameters.tauOil);
        break;
      case PARAMETER_TAU_AIR:
        BTserial.println("PARAMETER_TAU_AIR");
        BTserial.println(digifiz_parameters.tauAir);
        UIODserial.println(digifiz_parameters.tauAir);
        break;
      case PARAMETER_TAU_TANK:
        BTserial.println("PARAMETER_TAU_TANK");
        BTserial.println(digifiz_parameters.tauTank);
        UIODserial.println(digifiz_parameters.tauTank);
        break;
      case PARAMETER_MILEAGE:
        BTserial.println("PARAMETER_MILEAGE");
        BTserial.println(digifiz_parameters.mileage);
        UIODserial.println(digifiz_parameters.mileage);
        break;
      case PARAMETER_DAILY_MILEAGE:
        BTserial.println("PARAMETER_DAILY_MILEAGE");
        BTserial.println(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]);
        UIODserial.println(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]);
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
        BTserial.println("PARAMETER_AUTO_BRIGHTNESS");
        BTserial.println(digifiz_parameters.autoBrightness);
        UIODserial.println(digifiz_parameters.autoBrightness);
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
        BTserial.println("PARAMETER_BRIGHTNESS_LEVEL");
        BTserial.println(digifiz_parameters.brightnessLevel);
        UIODserial.println(digifiz_parameters.brightnessLevel);
        break;
      case PARAMETER_TANK_CAPACITY:
        BTserial.println("PARAMETER_TANK_CAPACITY");
        BTserial.println(digifiz_parameters.tankCapacity);
        UIODserial.println(digifiz_parameters.tankCapacity);
        break;
      case PARAMETER_MFA_STATE:
        BTserial.println("PARAMETER_MFA_STATE");
        BTserial.println(digifiz_parameters.mfaState);
        UIODserial.println(digifiz_parameters.mfaState);
        break;
      case PARAMETER_BUZZER_OFF:
        BTserial.println("PARAMETER_BUZZER_OFF");
        BTserial.println(digifiz_parameters.buzzerOff);
        UIODserial.println(digifiz_parameters.buzzerOff);
        break;
      case PARAMETER_MAX_RPM:
        BTserial.println("PARAMETER_MAX_RPM");
        BTserial.println(digifiz_parameters.maxRPM);
        UIODserial.println(digifiz_parameters.maxRPM);
        break;
      case PARAMETER_BACKLIGHT_ON:
        BTserial.println("PARAMETER_BACKLIGHT_ON");
        BTserial.println(digifiz_parameters.backlight_on);
        UIODserial.println(digifiz_parameters.backlight_on);
        break;
      default:
        break;
    }
  }
  saveParameters();
}


void protocolParse()
{
  /*if ((millis()-statusTime)>2000)
  {
    statusTime=millis();
    BTserial.println("Digifiz Status");
  }*/
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
        //BTserial.println("received");
        // constrain the values to 0 - 255
        parameter = constrain(parameter, 0, 255);
        //value = constrain(value, 0, 65535);
        if(value<0)
          value=0;
        processData(parameter,value);
      }
      while(BTserial.available() > 0) {
        char t = BTserial.read();
      }
   }
#ifdef USE_UIOD
   if (UIODserial.available() > 0) 
   {
      // look for the next valid integer in the incoming serial stream:
      int parameter = UIODserial.parseInt(); 
      // do it again:
      long value = UIODserial.parseInt(); 
      // look for the newline. That's the end of your
      // sentence:
      if (UIODserial.read() == '\n') 
      {
        // sends confirmation
        //BTserial.println("received");
        // constrain the values to 0 - 255
        parameter = constrain(parameter, 0, 255);
        //value = constrain(value, 0, 65535);
        if(value<0)
          value=0;
        processData(parameter,value);
      }
      while(UIODserial.available() > 0) {
        char t = UIODserial.read();
      }
   }
#endif
}
