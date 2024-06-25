#include "setup.h"
#include "protocol.h"
#include "mfa.h"
#include "adc.h"
#include "esp_log.h"
#include <string.h>

uint32_t statusTime;

extern float spd_m_speedometer;
extern float averageRPM;
extern digifiz_pars digifiz_parameters; 
#define MAX_SIZE 128
char ws_data_send[MAX_SIZE];
char tmp_buffer[12];  // uint8_t can be at most 3 digits + null terminator
char cmd_buffer[128];
char cmd_buffer_par[64];
char cmd_buffer_val[64];

void initComProtocol()
{
    memset(ws_data_send,0,sizeof(ws_data_send));
}

void changeBLEName()
{
    
}

void processGPIOPinsValue(long value)
{
    
}

void printLnCString(char* data)
{
    // Get the current length of ws_data_send
    size_t current_len = strlen(ws_data_send);
    size_t data_len = strlen(data);
    
    // Check if there is enough space to append data
    if (current_len + data_len < MAX_SIZE) {
        // Append the data
        strcat(ws_data_send, data);
    } else {
        // Handle the case where data does not fit
        printf("Error: Not enough space to append data\n");
    }
    printf("%s\n",data);
}

void printLnUINT8(uint8_t val)
{
    // Convert uint8_t to string
    snprintf(tmp_buffer, sizeof(tmp_buffer), "%u\n", val);
    
    // Append the string representation of val to ws_data_send
    printLnCString(tmp_buffer);
}

void printLnUINT32(uint32_t val)
{
    // Convert uint8_t to string
    snprintf(tmp_buffer, sizeof(tmp_buffer), "%lu\n", val);
    
    // Append the string representation of val to ws_data_send
    printLnCString(tmp_buffer);
}

void printLnFloat(float val)
{
    // Convert uint8_t to string
    snprintf(tmp_buffer, sizeof(tmp_buffer), "%0.3f\n", val);
    
    // Append the string representation of val to ws_data_send
    printLnCString(tmp_buffer);
}

void processData(int parameter,long value)
{
  printf("par:%d\n",parameter);
  printf("val:%lu\n",value);
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
        printLnCString("PARAMETER_RPMCOEFFICIENT\n");
        digifiz_parameters.rpmCoefficient = value;
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        printLnCString("PARAMETER_SPEEDCOEEFICIENT\n");
        digifiz_parameters.speedCoefficient = value;
        break;
      case PARAMETER_COOLANTTHERMISTORB:
        printLnCString("PARAMETER_COOLANTTHERMISTORB\n");
        digifiz_parameters.coolantThermistorB = value;
        break;  
      case PARAMETER_OILTHERMISTORB:
        printLnCString("PARAMETER_OILTHERMISTORB\n");
        digifiz_parameters.oilThermistorB = value;
        break;  
      case PARAMETER_AIRTHERMISTORB:
        printLnCString("PARAMETER_AIRTHERMISTORB\n");
        digifiz_parameters.airThermistorB = value;
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        printLnCString("PARAMETER_TANKMINRESISTANCE\n");
        digifiz_parameters.tankMinResistance = value;
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        printLnCString("PARAMETER_TANKMAXRESISTANCE\n");
        digifiz_parameters.tankMaxResistance = value;
        break; 
      case PARAMETER_TAU_COOLANT:
        printLnCString("PARAMETER_TAU_COOLANT\n");
        digifiz_parameters.tauCoolant = value;
        break;
      case PARAMETER_TAU_OIL:
        printLnCString("PARAMETER_TAU_OIL\n");
        digifiz_parameters.tauOil = value;
        break;
      case PARAMETER_TAU_AIR:
        printLnCString("PARAMETER_TAU_AIR\n");
        digifiz_parameters.tauAir = value;
        break;
      case PARAMETER_TAU_TANK:
        printLnCString("PARAMETER_TAU_TANK\n");
        digifiz_parameters.tauTank = value;
        break;
      case PARAMETER_MILEAGE:
        printLnCString("PARAMETER_MILEAGE\n");
        digifiz_parameters.mileage = (uint32_t)value*3600;
        break;
      case PARAMETER_DAILY_MILEAGE:
        printLnCString("PARAMETER_DAILY_MILEAGE\n");
        digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock] = value;
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
        printLnCString("PARAMETER_AUTO_BRIGHTNESS\n");
        digifiz_parameters.autoBrightness = value;
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
        printLnCString("PARAMETER_BRIGHTNESS_LEVEL\n");
        digifiz_parameters.brightnessLevel = value;
        break;
      case PARAMETER_TANK_CAPACITY:
        printLnCString("PARAMETER_TANK_CAPACITY\n");
        digifiz_parameters.tankCapacity = value;
        break;
      case PARAMETER_MFA_STATE:
        printLnCString("PARAMETER_MFA_STATE\n");
        digifiz_parameters.mfaState = value;
        break;
      case PARAMETER_BUZZER_OFF:
        printLnCString("PARAMETER_BUZZER_OFF\n");
        digifiz_parameters.buzzerOff = value;
        break;
      case PARAMETER_MAX_RPM:
        printLnCString("PARAMETER_MAX_RPM\n");
        digifiz_parameters.maxRPM = value;
        break;
      case PARAMETER_DOT_OFF:
        printLnCString("PARAMETER_DOT_OFF\n");
        digifiz_parameters.displayDot = value;
        break;
      case PARAMETER_BACKLIGHT_ON:
        printLnCString("PARAMETER_BACKLIGHT_ON\n");
        digifiz_parameters.backlight_on = value;
        break;
      case PARAMETER_M_D_FILTER:
        printLnCString("PARAMETER_M_D_FILTER\n");
        digifiz_parameters.medianDispFilterThreshold = value;
        break;
      case PARAMETER_COOLANT_MAX_R:
        printLnCString("PARAMETER_COOLANT_MAX_R\n");
        digifiz_parameters.coolantMax = value;
        break;
      case PARAMETER_COOLANT_MIN_R:
        printLnCString("PARAMETER_COOLANT_MIN_R\n");
        digifiz_parameters.coolantMin = value;
        break;
      case PARAMETER_COMMAND_MFA_RESET:
        printLnCString("PARAMETER_COMMAND_MFA_RESET\n");
	    pressMFAReset();
        break;
      case PARAMETER_COMMAND_MFA_MODE:
        printLnCString("PARAMETER_COMMAND_MFA_MODE\n");
        pressMFAMode();
        break;
      case PARAMETER_COMMAND_MFA_BLOCK:
        printLnCString("PARAMETER_COMMAND_MFA_BLOCK\n");
        pressMFABlock();
        break;
      case PARAMETER_MAINCOLOR_R:
        printLnCString("PARAMETER_MAINCOLOR_R\n");
        digifiz_parameters.mainc_r = value;
        break;
      case PARAMETER_MAINCOLOR_G:
        printLnCString("PARAMETER_MAINCOLOR_G\n");
        digifiz_parameters.mainc_g = value;
        break;
      case PARAMETER_MAINCOLOR_B:
        printLnCString("PARAMETER_MAINCOLOR_B\n");
        digifiz_parameters.mainc_b = value;
        break;
      case PARAMETER_UPTIME:
        printLnCString("PARAMETER_UPTIME\n");
        digifiz_parameters.uptime = value;
        break;  
      default:
        break;
    }
  }
  else
  {
    //commands
    if (par==PARAMETER_SET_HOUR)
    {
           
    }
    else
    {
      if (par==PARAMETER_SET_MINUTE)
      {
          
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
        printLnCString("PARAMETER_GET_ACCUMULATED_UPTIME\n");
        printLnFloat(digifiz_parameters.uptime);
        break;
      case PARAMETER_GET_COOLANT_TEMPERATURE:
        printLnCString("PARAMETER_GET_COOLANT_TEMPERATURE\n");
        printLnFloat(getCoolantTemperature());
        break;
      case PARAMETER_GET_AMBIENT_TEMPERATURE:
        printLnCString("PARAMETER_GET_AMBIENT_TEMPERATURE\n");
        printLnFloat(getAmbientTemperature());
        break;
      case PARAMETER_GET_OIL_TEMPERATURE:
        printLnCString("PARAMETER_GET_OIL_TEMPERATURE\n");
        printLnFloat(getOilTemperature());
        break;
      case PARAMETER_GET_FUEL_IN_TANK:
        printLnCString("PARAMETER_GET_FUEL_IN_TANK\n");
        printLnFloat(getLitresInTank());
        break;
      case PARAMETER_GET_SPEED:
        printLnCString("PARAMETER_GET_SPEED\n");
        printLnFloat(spd_m_speedometer);
        break;
      case PARAMETER_GET_RPM:
        printLnCString("PARAMETER_GET_RPM\n");
        printLnFloat(averageRPM);
        break;
      case PARAMETER_GET_DAY:
        printLnCString("PARAMETER_GET_DAY\n");
        // printLnFloat(myRTC.now().day());
        break;
      case PARAMETER_GET_MONTH:
        printLnCString("PARAMETER_GET_MONTH\n");
        // printLnFloat(myRTC.now().month());
        break;
      case PARAMETER_GET_YEAR:
        printLnCString("PARAMETER_GET_YEAR\n");
        // printLnFloat(myRTC.now().year());
        break;
      case PARAMETER_GET_HOUR:
        printLnCString("PARAMETER_GET_HOUR\n");
        // printLnFloat(myRTC.now().hour());
        break;
      case PARAMETER_GET_MINUTE:
        printLnCString("PARAMETER_GET_MINUTE\n");
        // printLnFloat(myRTC.now().minute());
        break;
      case PARAMETER_GET_GPIO_PINS:
        processGPIOPinsValue(value);
      default:
        break;
    }

    par-=PARAMETER_READ_ADDITION;
    
    //reads
    switch(par)
    {
      case PARAMETER_RPMCOEFFICIENT:
        printLnCString("PARAMETER_RPMCOEFFICIENT\n");
        printLnFloat(digifiz_parameters.rpmCoefficient);
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        printLnCString("PARAMETER_SPEEDCOEEFICIENT\n");
        printLnFloat(digifiz_parameters.speedCoefficient);
        break;
      case PARAMETER_COOLANTTHERMISTORB:
        printLnCString("PARAMETER_COOLANTTHERMISTORB\n");
        printLnFloat(digifiz_parameters.coolantThermistorB);
        break;  
      case PARAMETER_OILTHERMISTORB:
        printLnCString("PARAMETER_OILTHERMISTORB\n");
        printLnFloat(digifiz_parameters.oilThermistorB);
        break;  
      case PARAMETER_AIRTHERMISTORB:
        printLnCString("PARAMETER_AIRTHERMISTORB\n");
        printLnFloat(digifiz_parameters.airThermistorB);
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        printLnCString("PARAMETER_TANKMINRESISTANCE\n");
        printLnFloat(digifiz_parameters.tankMinResistance);
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        printLnCString("PARAMETER_TANKMAXRESISTANCE\n");
        printLnFloat(digifiz_parameters.tankMaxResistance);
        break; 
      case PARAMETER_TAU_COOLANT:
        printLnCString("PARAMETER_TAU_COOLANT\n");
        printLnFloat(digifiz_parameters.tauCoolant);
        break;
      case PARAMETER_TAU_OIL:
        printLnCString("PARAMETER_TAU_OIL\n");
        printLnUINT32(digifiz_parameters.tauOil);
        break;
      case PARAMETER_TAU_AIR:
        printLnCString("PARAMETER_TAU_AIR\n");
        printLnUINT32(digifiz_parameters.tauAir);
        break;
      case PARAMETER_TAU_TANK:
        printLnCString("PARAMETER_TAU_TANK\n");
        printLnUINT32(digifiz_parameters.tauTank);
        break;
      case PARAMETER_MILEAGE:
        printLnCString("PARAMETER_MILEAGE\n");
        printLnUINT32(digifiz_parameters.mileage);
        break;
      case PARAMETER_DAILY_MILEAGE:
        printLnCString("PARAMETER_DAILY_MILEAGE\n");
        printLnUINT32(digifiz_parameters.daily_mileage[digifiz_parameters.mfaBlock]);
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
        printLnCString("PARAMETER_AUTO_BRIGHTNESS\n");
        printLnUINT8(digifiz_parameters.autoBrightness);
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
        printLnCString("PARAMETER_BRIGHTNESS_LEVEL\n");
        printLnUINT8(digifiz_parameters.brightnessLevel);
        break;
      case PARAMETER_TANK_CAPACITY:
        printLnCString("PARAMETER_TANK_CAPACITY\n");
        printLnUINT8(digifiz_parameters.tankCapacity);
        break;
      case PARAMETER_MFA_STATE:
        printLnCString("PARAMETER_MFA_STATE\n");
        printLnUINT8(digifiz_parameters.mfaState);
        break;
      case PARAMETER_BUZZER_OFF:
        printLnCString("PARAMETER_BUZZER_OFF\n");
        printLnUINT8(digifiz_parameters.buzzerOff);
        break;
      case PARAMETER_MAX_RPM:
        printLnCString("PARAMETER_MAX_RPM\n");
        printLnUINT32(digifiz_parameters.maxRPM);
        break;
      case PARAMETER_BACKLIGHT_ON:
        printLnCString("PARAMETER_BACKLIGHT_ON\n");
        printLnUINT8(digifiz_parameters.backlight_on);
        break;
      default:
        break;
    }
  }
  //saveParameters();
}

void printAbout()
{
  printLnCString("Digifiz Replica by PHOL-LABS.\n");
  printLnCString("Fedor Zagumennov,\n");
  printLnCString("Pavel Myasnikov,\n");
  printLnCString("Cherry Fox / Duplux Indicators,\n");
  printLnCString("Egor Avramenko.\n");
}

void printHelp()
{
  printLnCString("Digifiz Replica by PHOL-LABS.\n");
  printLnCString("Your dashboard is:\n");
  if (digifiz_parameters.digifiz_options&OPTION_MFA_MANUFACTURER)
    printLnCString("MFA ON\n");
  else
    printLnCString("MFA OFF\n");

  if (digifiz_parameters.digifiz_options&OPTION_MILES)
    printLnCString("MPH\n");
  else
    printLnCString("KMH\n");
  if (digifiz_parameters.digifiz_options&OPTION_FAHRENHEIT)
    printLnCString("Fahrenheit\n");
  else
  {
    if (digifiz_parameters.digifiz_options&OPTION_KELVIN)
      printLnCString("Lelvin\n");
    else
      printLnCString("Celsium\n");
  }
  if (digifiz_parameters.digifiz_options&OPTION_GALLONS)
    printLnCString("Gallons\n");
  else
    printLnCString("Liters\n");
  printLnUINT32(digifiz_parameters.maxRPM);
  printLnCString(" RPM\n");
}

void clearProtocolBuffer(void)
{
    memset(ws_data_send,0,sizeof(ws_data_send));
}

void protocolParse(char* buf, uint8_t len)
{
    for (uint8_t i=0;i!=len;i++)
    {
        char c_char = buf[i];
        if (c_char == '\n')
        {
            //process and wait for new cmd
            memset(cmd_buffer_par,0,sizeof(cmd_buffer_par));
            memset(cmd_buffer_val,0,sizeof(cmd_buffer_val));
            uint8_t found_separator = 0;
            uint8_t separator_offset = 0;
            for (uint8_t j=0;j!=sizeof(cmd_buffer);j++)
            {
                if (cmd_buffer[j]==' ')
                {
                    found_separator = 1;
                    separator_offset = j;
                    break;
                }
            }
            if (found_separator)
            {
                int parameter_p = 0;
                long value_p = 0;
                memcpy(cmd_buffer_par, cmd_buffer, separator_offset);
                memcpy(cmd_buffer_val, cmd_buffer+separator_offset+1, strlen(cmd_buffer)-separator_offset-1);
                printf("cmd_buffer_par:%s\n",cmd_buffer_par);
                printf("cmd_buffer_val:%s\n",cmd_buffer_val);
                if (strcmp(cmd_buffer_par,"help")==0)
                {
                    printHelp();
                }
                else if (strcmp(cmd_buffer_par,"about")==0)
                {
                    printAbout();
                }
                else
                {
                    parameter_p = atoi(cmd_buffer);
                    value_p = atol(cmd_buffer_val);
                    if(value_p<0)
                        value_p=0;
                    processData(parameter_p, value_p);
                }
            }
            memset(cmd_buffer,0,sizeof(cmd_buffer));
        }
        else
        {
            cmd_buffer[i]=c_char;
        }
    }
    
}
