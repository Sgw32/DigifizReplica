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

  if ((protocol_locked)&&(par!=PARAMETER_PROTOCOL_UNLOCK))
     return;
  
  if (par<PARAMETER_READ_ADDITION)
  {
    switch(par)
    {
      case PARAMETER_RPMCOEFFICIENT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_RPMCOEFFICIENT");
        #endif
        digifiz_parameters.rpmCoefficient = value;
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SPEEDCOEEFICIENT");
        #endif
        digifiz_parameters.speedCoefficient = value;
        break;
      case PARAMETER_COOLANTTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANTTHERMISTORB");
        #endif
        digifiz_parameters.coolantThermistorB = value;
        break;  
      case PARAMETER_OILTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_OILTHERMISTORB");
        #endif
        digifiz_parameters.oilThermistorB = value;
        break;  
      case PARAMETER_AIRTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AIRTHERMISTORB");
        #endif
        digifiz_parameters.airThermistorB = value;
        break;  
      case PARAMETER_TANKMINRESISTANCE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMINRESISTANCE");
        #endif
        digifiz_parameters.tankMinResistance = value;
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMAXRESISTANCE");
        #endif
        digifiz_parameters.tankMaxResistance = value;
        break; 
      case PARAMETER_TAU_COOLANT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_COOLANT");
        #endif
        digifiz_parameters.tauCoolant = value;
        break;
      case PARAMETER_TAU_OIL:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_OIL");
        #endif
        digifiz_parameters.tauOil = value;
        break;
      case PARAMETER_TAU_AIR:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_AIR");
        #endif
        digifiz_parameters.tauAir = value;
        break;
      case PARAMETER_TAU_TANK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_TANK");
        #endif
        digifiz_parameters.tauTank = value;
        break;
      case PARAMETER_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MILEAGE");
        #endif
        digifiz_parameters.mileage = (uint32_t)value*3600;
        break;
      case PARAMETER_DAILY_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_DAILY_MILEAGE");
        #endif
        digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock] = value;
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AUTO_BRIGHTNESS");
        #endif
        digifiz_parameters.autoBrightness = value;
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BRIGHTNESS_LEVEL");
        #endif
        digifiz_parameters.brightnessLevel = value;
        break;
      case PARAMETER_TANK_CAPACITY:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANK_CAPACITY");
        #endif
        digifiz_parameters.tankCapacity = value;
        break;
      case PARAMETER_MFA_STATE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MFA_STATE");
        #endif
        digifiz_parameters.mfaState = value;
        break;
      case PARAMETER_BUZZER_OFF:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BUZZER_OFF");
        #endif
        digifiz_parameters.buzzerOff = value;
        break;
      case PARAMETER_MAX_RPM:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_RPM");
        #endif
        digifiz_parameters.maxRPM = value;
        break;
      case PARAMETER_DOT_OFF:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_DOT_OFF");
        #endif
        digifiz_parameters.displayDot = value;
        break;
      case PARAMETER_BACKLIGHT_ON:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BACKLIGHT_ON");
        #endif
        digifiz_parameters.backlight_on = value;
        break;
      case PARAMETER_M_D_FILTER:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_M_D_FILTER");
        #endif
        digifiz_parameters.medianDispFilterThreshold = value;
        break;
      case PARAMETER_COOLANT_MAX_R:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANT_MAX_R");
        #endif
        digifiz_parameters.coolantMaxResistance = value;
        break;
      case PARAMETER_COOLANT_MIN_R:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANT_MIN_R");
        #endif
        digifiz_parameters.coolantMinResistance = value;
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
      case PARAMETER_UPTIME:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_UPTIME");
        #endif
        digifiz_parameters.uptime = value;
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
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_GET_ACCUMULATED_UPTIME");
        BTserial.println(digifiz_parameters.uptime);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.uptime);
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
      case PARAMETER_PROTOCOL_LOCK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_PROTOCOL_LOCK");
        #endif
        #ifdef USE_UIOD
        #endif
        if (value==123)
           protocol_locked = 1;
        break;        
      case PARAMETER_PROTOCOL_UNLOCK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_PROTOCOL_UNLOCK");
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
        BTserial.println(digifiz_parameters.rpmCoefficient);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.rpmCoefficient);
        #endif
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_SPEEDCOEEFICIENT");
        BTserial.println(digifiz_parameters.speedCoefficient);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.speedCoefficient);
        #endif
        break;
      case PARAMETER_COOLANTTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_COOLANTTHERMISTORB");
        BTserial.println(digifiz_parameters.coolantThermistorB);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.coolantThermistorB);
        #endif
        break;  
      case PARAMETER_OILTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_OILTHERMISTORB");
        BTserial.println(digifiz_parameters.oilThermistorB);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.oilThermistorB);
        #endif
        break;  
      case PARAMETER_AIRTHERMISTORB:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AIRTHERMISTORB");
        BTserial.println(digifiz_parameters.airThermistorB);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.airThermistorB);
        #endif
        break;  
      case PARAMETER_TANKMINRESISTANCE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMINRESISTANCE");
        BTserial.println(digifiz_parameters.tankMinResistance);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tankMinResistance);
        #endif
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANKMAXRESISTANCE");
        BTserial.println(digifiz_parameters.tankMaxResistance);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tankMaxResistance);
        #endif
        break; 
      case PARAMETER_TAU_COOLANT:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_COOLANT");
        BTserial.println(digifiz_parameters.tauCoolant);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauCoolant);
        #endif
        break;
      case PARAMETER_TAU_OIL:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_OIL");
        BTserial.println(digifiz_parameters.tauOil);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauOil);
        #endif
        break;
      case PARAMETER_TAU_AIR:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_AIR");
        BTserial.println(digifiz_parameters.tauAir);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauAir);
        #endif
        break;
      case PARAMETER_TAU_TANK:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TAU_TANK");
        BTserial.println(digifiz_parameters.tauTank);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tauTank);
        #endif
        break;
      case PARAMETER_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MILEAGE");
        BTserial.println(digifiz_parameters.mileage);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.mileage);
        #endif
        break;
      case PARAMETER_DAILY_MILEAGE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_DAILY_MILEAGE");
        BTserial.println(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]);
        #endif
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_AUTO_BRIGHTNESS");
        BTserial.println(digifiz_parameters.autoBrightness);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.autoBrightness);
        #endif
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BRIGHTNESS_LEVEL");
        BTserial.println(digifiz_parameters.brightnessLevel);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.brightnessLevel);
        #endif
        break;
      case PARAMETER_TANK_CAPACITY:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_TANK_CAPACITY");
        BTserial.println(digifiz_parameters.tankCapacity);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.tankCapacity);
        #endif
        break;
      case PARAMETER_MFA_STATE:
      #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MFA_STATE");
        BTserial.println(digifiz_parameters.mfaState);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.mfaState);
        #endif
        break;
      case PARAMETER_BUZZER_OFF:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BUZZER_OFF");
        BTserial.println(digifiz_parameters.buzzerOff);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.buzzerOff);
        #endif
        break;
      case PARAMETER_MAX_RPM:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_MAX_RPM");
        BTserial.println(digifiz_parameters.maxRPM);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.maxRPM);
        #endif
        break;
      case PARAMETER_BACKLIGHT_ON:
        #ifdef USE_BTSERIAL
        BTserial.println("PARAMETER_BACKLIGHT_ON");
        BTserial.println(digifiz_parameters.backlight_on);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.backlight_on);
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

void printHelp()
{
  #ifdef USE_BTSERIAL
  BTserial.println("Digifiz Replica by PHOL-LABS.");
  BTserial.println("Your dashboard is:");
  if (digifiz_parameters.digifiz_options&OPTION_MFA_MANUFACTURER)
    BTserial.println("MFA ON");
  else
    BTserial.println("MFA OFF");

  if (digifiz_parameters.digifiz_options&OPTION_MILES)
    BTserial.println("MPH");
  else
    BTserial.println("KMH");
  if (digifiz_parameters.digifiz_options&OPTION_FAHRENHEIT)
    BTserial.println("Fahrenheit");
  else
  {
    if (digifiz_parameters.digifiz_options&OPTION_KELVIN)
      BTserial.println("Lelvin");
    else
      BTserial.println("Celsium");
  }
  if (digifiz_parameters.digifiz_options&OPTION_GALLONS)
    BTserial.println("Gallons");
  else
    BTserial.println("Liters");
  
  BTserial.print(digifiz_parameters.maxRPM);
  BTserial.println(" RPM");
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
