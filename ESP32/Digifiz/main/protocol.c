#include "setup.h"
#include "protocol.h"
#include "mfa.h"
#include "adc.h"
#include "esp_log.h"
#include <string.h>
#include "digifiz_time.h"
#include "vehicle_data.h"
#include "display_next.h"
#include <stdlib.h>
#include <ctype.h>

uint32_t statusTime;

extern float spd_m_speedometer;
extern float averageRPM;
#define MAX_SIZE 512
char ws_data_send[MAX_SIZE];
char tmp_buffer[12];  // uint8_t can be at most 3 digits + null terminator
char cmd_buffer[128];
char cmd_buffer_par[64];
char cmd_buffer_val[64];

// Mapping array
static const ParameterMap parameter_map[] = {
    {"PARAMETER_ZERO_RESERVED", PARAMETER_ZERO_RESERVED},
    {"PARAMETER_SPEEDCOEEFICIENT", PARAMETER_SPEEDCOEEFICIENT},
    {"PARAMETER_COOLANTTHERMISTORB", PARAMETER_COOLANTTHERMISTORB},
    {"PARAMETER_OILTHERMISTORB", PARAMETER_OILTHERMISTORB},
    {"PARAMETER_AIRTHERMISTORB", PARAMETER_AIRTHERMISTORB},
    {"PARAMETER_TANKMINRESISTANCE", PARAMETER_TANKMINRESISTANCE},
    {"PARAMETER_TANKMAXRESISTANCE", PARAMETER_TANKMAXRESISTANCE},
    {"PARAMETER_TAU_COOLANT", PARAMETER_TAU_COOLANT},
    {"PARAMETER_TAU_OIL", PARAMETER_TAU_OIL},
    {"PARAMETER_TAU_AIR", PARAMETER_TAU_AIR},
    {"PARAMETER_TAU_TANK", PARAMETER_TAU_TANK},
    {"PARAMETER_MILEAGE", PARAMETER_MILEAGE},
    {"PARAMETER_DAILY_MILEAGE", PARAMETER_DAILY_MILEAGE},
    {"PARAMETER_AUTO_BRIGHTNESS", PARAMETER_AUTO_BRIGHTNESS},
    {"PARAMETER_BRIGHTNESS_LEVEL", PARAMETER_BRIGHTNESS_LEVEL},
    {"PARAMETER_TANK_CAPACITY", PARAMETER_TANK_CAPACITY},
    {"PARAMETER_MFA_STATE", PARAMETER_MFA_STATE},
    {"PARAMETER_BUZZER_OFF", PARAMETER_BUZZER_OFF},
    {"PARAMETER_MAX_RPM", PARAMETER_MAX_RPM},
    {"PARAMETER_NORMAL_RESISTANCE_COOLANT", PARAMETER_NORMAL_RESISTANCE_COOLANT},
    {"PARAMETER_NORMAL_RESISTANCE_OIL", PARAMETER_NORMAL_RESISTANCE_OIL},
    {"PARAMETER_NORMAL_RESISTANCE_AMB", PARAMETER_NORMAL_RESISTANCE_AMB},
    {"PARAMETER_RPMCOEFFICIENT", PARAMETER_RPMCOEFFICIENT},
    {"PARAMETER_DOT_OFF", PARAMETER_DOT_OFF},
    {"PARAMETER_BACKLIGHT_ON", PARAMETER_BACKLIGHT_ON},
    {"PARAMETER_M_D_FILTER", PARAMETER_M_D_FILTER},
    {"PARAMETER_COOLANT_MAX_R", PARAMETER_COOLANT_MAX_R},
    {"PARAMETER_COOLANT_MIN_R", PARAMETER_COOLANT_MIN_R},
    {"PARAMETER_COMMAND_MFA_RESET", PARAMETER_COMMAND_MFA_RESET},
    {"PARAMETER_COMMAND_MFA_MODE", PARAMETER_COMMAND_MFA_MODE},
    {"PARAMETER_COMMAND_MFA_BLOCK", PARAMETER_COMMAND_MFA_BLOCK},
    {"PARAMETER_MAINCOLOR_R", PARAMETER_MAINCOLOR_R},
    {"PARAMETER_MAINCOLOR_G", PARAMETER_MAINCOLOR_G},
    {"PARAMETER_MAINCOLOR_B", PARAMETER_MAINCOLOR_B},
    {"PARAMETER_BACKCOLOR_R", PARAMETER_BACKCOLOR_R},
    {"PARAMETER_BACKCOLOR_G", PARAMETER_BACKCOLOR_G},
    {"PARAMETER_BACKCOLOR_B", PARAMETER_BACKCOLOR_B},
    {"PARAMETER_RPM_FILTER", PARAMETER_RPM_FILTER},
    {"PARAMETER_SPEED_FILTER", PARAMETER_SPEED_FILTER},
    {"PARAMETER_SET_FUEL_CALC_FUNCTION", PARAMETER_SET_FUEL_CALC_FUNCTION},
    {"PARAMETER_SET_RPM_OPTIONS", PARAMETER_SET_RPM_OPTIONS},
    {"PARAMETER_SET_TEMP_OPTIONS", PARAMETER_SET_TEMP_OPTIONS},
    {"PARAMETER_SET_SIGNAL_OPTIONS", PARAMETER_SET_SIGNAL_OPTIONS},
    {"PARAMETER_PULLUP_RESISTANCE_COOLANT", PARAMETER_PULLUP_RESISTANCE_COOLANT},
    {"PARAMETER_PULLUP_RESISTANCE_OIL", PARAMETER_PULLUP_RESISTANCE_OIL},
    {"PARAMETER_PULLUP_RESISTANCE_AMB", PARAMETER_PULLUP_RESISTANCE_AMB},
    {"PARAMETER_UPTIME", PARAMETER_UPTIME},
    {"PARAMETER_READ_ADDITION", PARAMETER_READ_ADDITION},
    {"PARAMETER_SET_HOUR", PARAMETER_SET_HOUR},
    {"PARAMETER_SET_MINUTE", PARAMETER_SET_MINUTE},
    {"PARAMETER_RESET_DAILY_MILEAGE", PARAMETER_RESET_DAILY_MILEAGE},
    {"PARAMETER_RESET_DIGIFIZ", PARAMETER_RESET_DIGIFIZ},
    {"PARAMETER_GET_ACCUMULATED_UPTIME", PARAMETER_GET_ACCUMULATED_UPTIME},
    {"PARAMETER_GET_COOLANT_TEMPERATURE", PARAMETER_GET_COOLANT_TEMPERATURE},
    {"PARAMETER_GET_OIL_TEMPERATURE", PARAMETER_GET_OIL_TEMPERATURE},
    {"PARAMETER_GET_AMBIENT_TEMPERATURE", PARAMETER_GET_AMBIENT_TEMPERATURE},
    {"PARAMETER_GET_FUEL_IN_TANK", PARAMETER_GET_FUEL_IN_TANK},
    {"PARAMETER_GET_SPEED", PARAMETER_GET_SPEED},
    {"PARAMETER_GET_RPM", PARAMETER_GET_RPM},
    {"PARAMETER_SET_DAY", PARAMETER_SET_DAY},
    {"PARAMETER_SET_MONTH", PARAMETER_SET_MONTH},
    {"PARAMETER_SET_YEAR", PARAMETER_SET_YEAR},
    {"PARAMETER_GET_DAY", PARAMETER_GET_DAY},
    {"PARAMETER_GET_MONTH", PARAMETER_GET_MONTH},
    {"PARAMETER_GET_YEAR", PARAMETER_GET_YEAR},
    {"PARAMETER_GET_HOUR", PARAMETER_GET_HOUR},
    {"PARAMETER_GET_MINUTE", PARAMETER_GET_MINUTE},
    {"PARAMETER_GET_GPIO_PINS", PARAMETER_GET_GPIO_PINS},
    {"PARAMETER_MEMORY_LOCK", PARAMETER_MEMORY_LOCK},
    {"PARAMETER_MEMORY_UNLOCK", PARAMETER_MEMORY_UNLOCK},
    {"PARAMETER_TOGGLE_MILES", PARAMETER_TOGGLE_MILES},
    {"PARAMETER_TOGGLE_GALLONS", PARAMETER_TOGGLE_GALLONS},
    {"PARAMETER_TOGGLE_FAHRENHEIT", PARAMETER_TOGGLE_FAHRENHEIT},
    {"PARAMETER_TOGGLE_TOUCH_SENSOR", PARAMETER_TOGGLE_TOUCH_SENSOR},
    {"PARAMETER_SAVE_PARAMS", PARAMETER_SAVE_PARAMS},
};

static void printAbout()
{
  printLnCString("Digifiz Replica by PHOL-LABS.\n");
  printLnCString("Fedor Zagumennov,\n");
  printLnCString("Pavel Myasnikov,\n");
  printLnCString("Cherry Fox / Duplux Indicators,\n");
  printLnCString("Egor Avramenko.\n");
}

static void setMainTemplateColor(char* color_values)
{
  uint8_t r = 0, g = 0, b = 0;
  // Parse the color values (format: "R G B")
  if (sscanf(color_values, "%hhu %hhu %hhu", &r, &g, &b) == 3) {
      // Successfully parsed RGB values
      printLnCString("Successfully set new colors.\n");
      printf("Received RGB values: R=%u, G=%u, B=%u\n", r, g, b);
      digifiz_parameters.mainc_r.value = r;
      digifiz_parameters.mainc_g.value = g;
      digifiz_parameters.mainc_b.value = b;
  } else {
      // Failed to parse the values
      printLnCString("Invalid color format.\n");
      printLnCString(color_values);
      printf("Invalid color format: %s\n", color_values);
  }
  compileColorScheme();
}

static void setBacklightTemplateColor(char* color_values)
{
  uint8_t r = 0, g = 0, b = 0;
  // Parse the color values (format: "R G B")
  if (sscanf(color_values, "%hhu %hhu %hhu", &r, &g, &b) == 3) {
      // Successfully parsed RGB values
      printLnCString("Successfully set new colors.\n");
      printf("Received RGB values: R=%u, G=%u, B=%u\n", r, g, b);
      digifiz_parameters.backc_r.value = r;
      digifiz_parameters.backc_g.value = g;
      digifiz_parameters.backc_b.value = b;
  } else {
      // Failed to parse the values
      printLnCString("Invalid color format.\n");
      printLnCString(color_values);
      printf("Invalid color format: %s\n", color_values);
  }
  compileColorScheme();
}

static void printADC()
{
  printLnCString("ADC:\n");
  //printLnCString("getCoolantRawADCVal:");
  printLnFloat((float)getCoolantRawADCVal());
  //printLnCString("getFuelRawADCVal:");
  printLnFloat((float)getFuelRawADCVal());
  //printLnCString("getLightRawADCVal:");
  printLnFloat((float)getLightRawADCVal());
  //printLnCString("getAmbTempRawADCVal:");
  printLnFloat((float)getAmbTempRawADCVal());
  //printLnCString("getOilTempRawADCVal:");
  printLnFloat((float)getOilTempRawADCVal());
  //rintLnCString("getIntakePressRawADCVal:");
  printLnFloat((float)getIntakePressRawADCVal());
  //printLnCString("getFuelPressRawADCVal:");
  printLnFloat((float)getFuelPressRawADCVal());
}

static void printStatusJSON()
{
  update_json_string();
  printLnCString(g_jsonString);
}

static void printHelp()
{
  printLnCString("Digifiz Replica by PHOL-LABS.\n");
  printLnCString("Your dashboard is:\n");
  if (digifiz_parameters.option_mfa_manufacturer.value)
    printLnCString("MFA ON\n");
  else
    printLnCString("MFA OFF\n");

  if (digifiz_parameters.option_miles.value)
    printLnCString("MPH\n");
  else
    printLnCString("KMH\n");
  if (digifiz_parameters.option_fahrenheit.value)
    printLnCString("Fahrenheit\n");
  else
  {
    if (digifiz_parameters.option_kelvin.value)
      printLnCString("Lelvin\n");
    else
      printLnCString("Celsium\n");
  }
  if (digifiz_parameters.option_gallons.value)
    printLnCString("Gallons\n");
  else
    printLnCString("Liters\n");
  printLnUINT32(digifiz_parameters.maxRPM.value);
  printLnCString(" RPM\n");
}

// Function to map parameter name to enum value
static int getParameterValue(const char *name, int *value) {
    size_t i;
    size_t map_size = sizeof(parameter_map) / sizeof(parameter_map[0]);
    for (i = 0; i < map_size; i++) {
        if (strcmp(parameter_map[i].name, name) == 0) {
            *value = parameter_map[i].value;
            return 1; // Success
        }
    }
    return 0; // Not found
}

// Function to check if a string is a valid integer
static int is_number(const char *str) {
    if (*str == '\0') return 0;
    if (*str == '-' || *str == '+') str++;
    while (*str) {
      if (((unsigned char)*str)!=' ')
      {
        if (!isdigit((unsigned char)*str)) return 0;
      }
      str++;
    }
    return 1;
}

void initComProtocol()
{
    memset(ws_data_send,0,sizeof(ws_data_send));
}

void changeBLEName()
{
    //We do not have BLE, to be deleted
}

void processGPIOPinsValue(long value)
{
    //TODO implement
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
    //printf("%s\n",data);
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
            printLnCString("PARAMETER_RESET_DIGIFIZ\n");
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
        digifiz_parameters.rpmCoefficient.value = value;
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        printLnCString("PARAMETER_SPEEDCOEEFICIENT\n");
        digifiz_parameters.speedCoefficient.value = value;
        break;
      case PARAMETER_COOLANTTHERMISTORB:
        printLnCString("PARAMETER_COOLANTTHERMISTORB\n");
        digifiz_parameters.coolantThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_OILTHERMISTORB:
        printLnCString("PARAMETER_OILTHERMISTORB\n");
        digifiz_parameters.oilThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_AIRTHERMISTORB:
        printLnCString("PARAMETER_AIRTHERMISTORB\n");
        digifiz_parameters.airThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        printLnCString("PARAMETER_TANKMINRESISTANCE\n");
        digifiz_parameters.tankMinResistance.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        printLnCString("PARAMETER_TANKMAXRESISTANCE\n");
        digifiz_parameters.tankMaxResistance.value = value;
        updateADCSettings();
        break; 
      case PARAMETER_TAU_COOLANT:
        printLnCString("PARAMETER_TAU_COOLANT\n");
        digifiz_parameters.tauCoolant.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_OIL:
        printLnCString("PARAMETER_TAU_OIL\n");
        digifiz_parameters.tauOil.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_AIR:
        printLnCString("PARAMETER_TAU_AIR\n");
        digifiz_parameters.tauAir.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_TANK:
        printLnCString("PARAMETER_TAU_TANK\n");
        digifiz_parameters.tauTank.value = value;
        updateADCSettings();
        break;
      case PARAMETER_MILEAGE:
        printLnCString("PARAMETER_MILEAGE\n");
        digifiz_status.mileage = (uint32_t)value*3600;
        break;
      case PARAMETER_DAILY_MILEAGE:
        printLnCString("PARAMETER_DAILY_MILEAGE\n");
        digifiz_status.daily_mileage[digifiz_parameters.mfaBlock.value] = value;
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
        printLnCString("PARAMETER_AUTO_BRIGHTNESS\n");
        digifiz_parameters.autoBrightness.value = value;
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
        printLnCString("PARAMETER_BRIGHTNESS_LEVEL\n");
        digifiz_parameters.brightnessLevel.value = value;
        break;
      case PARAMETER_TANK_CAPACITY:
        printLnCString("PARAMETER_TANK_CAPACITY\n");
        digifiz_parameters.tankCapacity.value = value;
        updateADCSettings();
        break;
      case PARAMETER_MFA_STATE:
        printLnCString("PARAMETER_MFA_STATE\n");
        digifiz_parameters.mfaState.value = value;
        break;
      case PARAMETER_BUZZER_OFF:
        printLnCString("PARAMETER_BUZZER_OFF\n");
        digifiz_parameters.buzzerOff.value = value;
        break;
      case PARAMETER_MAX_RPM:
        printLnCString("PARAMETER_MAX_RPM\n");
        digifiz_parameters.maxRPM.value = value;
        break;
      case PARAMETER_NORMAL_RESISTANCE_COOLANT:
        printLnCString("PARAMETER_NORMAL_RESISTANCE_COOLANT\n");
        digifiz_parameters.coolantThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_NORMAL_RESISTANCE_OIL:
        printLnCString("PARAMETER_NORMAL_RESISTANCE_OIL\n");
        digifiz_parameters.oilThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_NORMAL_RESISTANCE_AMB:
        printLnCString("PARAMETER_NORMAL_RESISTANCE_AMB\n");
        digifiz_parameters.ambThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_DOT_OFF:
        printLnCString("PARAMETER_DOT_OFF\n");
        digifiz_parameters.displayDot.value = value;
        break;
      case PARAMETER_BACKLIGHT_ON:
        printLnCString("PARAMETER_BACKLIGHT_ON\n");
        digifiz_parameters.backlight_on.value = value;
        break;
      case PARAMETER_M_D_FILTER:
        printLnCString("PARAMETER_M_D_FILTER\n");
        digifiz_parameters.medianDispFilterThreshold.value = value;
        break;
      case PARAMETER_COOLANT_MAX_R:
        printLnCString("PARAMETER_COOLANT_MAX_R\n");
        digifiz_parameters.coolantMax.value = value;
        updateADCSettings();
        break;
      case PARAMETER_COOLANT_MIN_R:
        printLnCString("PARAMETER_COOLANT_MIN_R\n");
        digifiz_parameters.coolantMin.value = value;
        updateADCSettings();
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
        digifiz_parameters.mainc_r.value = value;
        break;
      case PARAMETER_MAINCOLOR_G:
        printLnCString("PARAMETER_MAINCOLOR_G\n");
        digifiz_parameters.mainc_g.value = value;
        break;
      case PARAMETER_MAINCOLOR_B:
        printLnCString("PARAMETER_MAINCOLOR_B\n");
        digifiz_parameters.mainc_b.value = value;
        break;
      case PARAMETER_BACKCOLOR_R:
        printLnCString("PARAMETER_BACKCOLOR_R\n");
        digifiz_parameters.backc_r.value = value;
        break;
      case PARAMETER_BACKCOLOR_G:
        printLnCString("PARAMETER_BACKCOLOR_G\n");
        digifiz_parameters.backc_g.value = value;
        break;
      case PARAMETER_BACKCOLOR_B:
        printLnCString("PARAMETER_BACKCOLOR_B\n");
        digifiz_parameters.backc_b.value = value;
        break;
      case PARAMETER_RPM_FILTER:
        printLnCString("PARAMETER_RPM_FILTER\n");
        digifiz_parameters.rpmFilterK.value = value;
        break;
      case PARAMETER_SET_FUEL_CALC_FUNCTION:
        printLnCString("PARAMETER_SET_FUEL_CALC_FUNCTION\n");
        // digifiz_parameters.option_linear_fuel.value = value&1;
        digifiz_parameters.option_linear_fuel.value = value&1;
        break;
      case PARAMETER_SET_RPM_OPTIONS:
        printLnCString("PARAMETER_SET_RPM_OPTIONS\n");
        digifiz_parameters.rpmOptions_redline_segments.value = value&31;
        digifiz_parameters.rpmOptions_diesel_line.value = value&32;
        break;
      case PARAMETER_SET_TEMP_OPTIONS:
        printLnCString("PARAMETER_SET_TEMP_OPTIONS\n");
        digifiz_parameters.tempOptions_red_segments.value = value&3;
        digifiz_parameters.tempOptions_sensor_connected_ind.value = value&4;
        digifiz_parameters.tempOptions_alarm_function.value = value&8;
        if ((value&16)!=digifiz_parameters.tempOptions_oil_atten.value)
        {
          digifiz_parameters.tempOptions_oil_atten.value = value&16;
          reconfigOilChannel();
        }
        break;
      case PARAMETER_SET_SIGNAL_OPTIONS:
        printLnCString("PARAMETER_SET_SIGNAL_OPTIONS\n");
        digifiz_parameters.signalOptions_use_blink_alt_in.value = value&1;
        digifiz_parameters.signalOptions_enable_touch_sensor.value = value&2;
        digifiz_parameters.signalOptions_invert_light_input.value = value&4;
        digifiz_parameters.signalOptions_enable_consumption_sensor.value = value&8;
        break;
      case PARAMETER_PULLUP_RESISTANCE_COOLANT:
        printLnCString("PARAMETER_PULLUP_RESISTANCE_COOLANT\n");
        digifiz_parameters.coolantThermistorPullUpRes.value = value;
        break;
      case PARAMETER_PULLUP_RESISTANCE_OIL:
        printLnCString("PARAMETER_PULLUP_RESISTANCE_OIL\n");
        digifiz_parameters.oilThermistorPullUpRes.value = value;
        break;
      case PARAMETER_PULLUP_RESISTANCE_AMB:
        printLnCString("PARAMETER_PULLUP_RESISTANCE_AMB\n");
        digifiz_parameters.ambThermistorPullUpRes.value = value;
        break;
      case PARAMETER_UPTIME:
        printLnCString("PARAMETER_UPTIME\n");
        digifiz_status.uptime = value;
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
      printLnCString("PARAMETER_SET_HOUR\n");
      set_hour(value);
    }
    else if (par==PARAMETER_TOGGLE_MILES)
    {
      printLnCString("PARAMETER_TOGGLE_MILES\n");
      if (!(digifiz_parameters.option_miles.value))
        digifiz_parameters.option_miles.value = 1;
      else
        digifiz_parameters.option_miles.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_GALLONS)
    {
      printLnCString("PARAMETER_TOGGLE_GALLONS\n");
      if (!(digifiz_parameters.option_gallons.value))
        digifiz_parameters.option_gallons.value = 1;
      else
        digifiz_parameters.option_gallons.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_FAHRENHEIT)
    {
      printLnCString("PARAMETER_TOGGLE_FAHRENHEIT\n");
      if (!(digifiz_parameters.option_fahrenheit.value))
        digifiz_parameters.option_fahrenheit.value = 1;
      else
        digifiz_parameters.option_fahrenheit.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_TOUCH_SENSOR)
    {
      printLnCString("PARAMETER_TOGGLE_TOUCH_SENSOR\n");
      if (!(digifiz_parameters.signalOptions_enable_touch_sensor.value))
        digifiz_parameters.signalOptions_enable_touch_sensor.value = 1;
      else
        digifiz_parameters.signalOptions_enable_touch_sensor.value = 0;
    }
    else if (par==PARAMETER_SAVE_PARAMS)
    {
      printLnCString("PARAMETER_SAVE_PARAMS\n");
      saveParameters();
    }
    else
    {
      if (par==PARAMETER_SET_MINUTE)
      {
        printLnCString("PARAMETER_SET_MINUTE\n");
        set_minute(value);
      }
      else
      {
        if (par==PARAMETER_RESET_DAILY_MILEAGE)
        {
          printLnCString("PARAMETER_RESET_DAILY_MILEAGE\n");
          digifiz_status.daily_mileage[digifiz_parameters.mfaBlock.value] = 0;
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
        printLnFloat(digifiz_status.uptime);
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
        printLnFloat(digifiz_parameters.rpmCoefficient.value);
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        printLnCString("PARAMETER_SPEEDCOEEFICIENT\n");
        printLnFloat(digifiz_parameters.speedCoefficient.value);
        break;
      case PARAMETER_COOLANTTHERMISTORB:
        printLnCString("PARAMETER_COOLANTTHERMISTORB\n");
        printLnFloat(digifiz_parameters.coolantThermistorB.value);
        break;  
      case PARAMETER_OILTHERMISTORB:
        printLnCString("PARAMETER_OILTHERMISTORB\n");
        printLnFloat(digifiz_parameters.oilThermistorB.value);
        break;  
      case PARAMETER_AIRTHERMISTORB:
        printLnCString("PARAMETER_AIRTHERMISTORB\n");
        printLnFloat(digifiz_parameters.airThermistorB.value);
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        printLnCString("PARAMETER_TANKMINRESISTANCE\n");
        printLnFloat(digifiz_parameters.tankMinResistance.value);
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        printLnCString("PARAMETER_TANKMAXRESISTANCE\n");
        printLnFloat(digifiz_parameters.tankMaxResistance.value);
        break; 
      case PARAMETER_TAU_COOLANT:
        printLnCString("PARAMETER_TAU_COOLANT\n");
        printLnFloat(digifiz_parameters.tauCoolant.value);
        break;
      case PARAMETER_TAU_OIL:
        printLnCString("PARAMETER_TAU_OIL\n");
        printLnUINT32(digifiz_parameters.tauOil.value);
        break;
      case PARAMETER_TAU_AIR:
        printLnCString("PARAMETER_TAU_AIR\n");
        printLnUINT32(digifiz_parameters.tauAir.value);
        break;
      case PARAMETER_TAU_TANK:
        printLnCString("PARAMETER_TAU_TANK\n");
        printLnUINT32(digifiz_parameters.tauTank.value);
        break;
      case PARAMETER_MILEAGE:
        printLnCString("PARAMETER_MILEAGE\n");
        printLnUINT32(digifiz_status.mileage);
        break;
      case PARAMETER_DAILY_MILEAGE:
        printLnCString("PARAMETER_DAILY_MILEAGE\n");
        printLnUINT32(digifiz_status.daily_mileage[digifiz_parameters.mfaBlock.value]);
        break;
      case PARAMETER_AUTO_BRIGHTNESS:
        printLnCString("PARAMETER_AUTO_BRIGHTNESS\n");
        printLnUINT8(digifiz_parameters.autoBrightness.value);
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
        printLnCString("PARAMETER_BRIGHTNESS_LEVEL\n");
        printLnUINT8(digifiz_parameters.brightnessLevel.value);
        break;
      case PARAMETER_TANK_CAPACITY:
        printLnCString("PARAMETER_TANK_CAPACITY\n");
        printLnUINT8(digifiz_parameters.tankCapacity.value);
        break;
      case PARAMETER_MFA_STATE:
        printLnCString("PARAMETER_MFA_STATE\n");
        printLnUINT8(digifiz_parameters.mfaState.value);
        break;
      case PARAMETER_BUZZER_OFF:
        printLnCString("PARAMETER_BUZZER_OFF\n");
        printLnUINT8(digifiz_parameters.buzzerOff.value);
        break;
      case PARAMETER_MAX_RPM:
        printLnCString("PARAMETER_MAX_RPM\n");
        printLnUINT32(digifiz_parameters.maxRPM.value);
        break;
      case PARAMETER_NORMAL_RESISTANCE_COOLANT:
        printLnCString("PARAMETER_NORMAL_RESISTANCE_COOLANT\n");
        printLnUINT32(digifiz_parameters.coolantThermistorDefRes.value);
        break;
      case PARAMETER_NORMAL_RESISTANCE_OIL:
        printLnCString("PARAMETER_NORMAL_RESISTANCE_OIL\n");
        printLnUINT32(digifiz_parameters.oilThermistorDefRes.value);
        break;
      case PARAMETER_NORMAL_RESISTANCE_AMB:
        printLnCString("PARAMETER_NORMAL_RESISTANCE_AMB\n");
        printLnUINT32(digifiz_parameters.ambThermistorDefRes.value);
        break;
      case PARAMETER_DOT_OFF:
        printLnCString("PARAMETER_DOT_OFF\n");
        printLnUINT32(digifiz_parameters.displayDot.value);
        break;
      case PARAMETER_BACKLIGHT_ON:
        printLnCString("PARAMETER_BACKLIGHT_ON\n");
        printLnUINT8(digifiz_parameters.backlight_on.value);
        break;
      case PARAMETER_M_D_FILTER:
        printLnCString("PARAMETER_M_D_FILTER\n");
        printLnUINT8(digifiz_parameters.medianDispFilterThreshold.value);
        break;
      case PARAMETER_COOLANT_MAX_R:
        printLnCString("PARAMETER_COOLANT_MAX_R\n");
        printLnUINT8(digifiz_parameters.coolantMax.value);
        break;
      case PARAMETER_COOLANT_MIN_R:
        printLnCString("PARAMETER_COOLANT_MIN_R\n");
        printLnUINT8(digifiz_parameters.coolantMin.value);
        break;
      case PARAMETER_MAINCOLOR_R:
        printLnCString("PARAMETER_MAINCOLOR_R\n");
        printLnUINT8(digifiz_parameters.mainc_r.value);
        compileColorScheme();
        break;
      case PARAMETER_MAINCOLOR_G:
        printLnCString("PARAMETER_MAINCOLOR_G\n");
        printLnUINT8(digifiz_parameters.mainc_g.value);
        compileColorScheme();
        break;
      case PARAMETER_MAINCOLOR_B:
        printLnCString("PARAMETER_MAINCOLOR_B\n");
        printLnUINT8(digifiz_parameters.mainc_b.value);
        compileColorScheme();
        break;
      case PARAMETER_BACKCOLOR_R:
        printLnCString("PARAMETER_BACKCOLOR_R\n");
        printLnUINT8(digifiz_parameters.backc_r.value);
        break;
      case PARAMETER_BACKCOLOR_G:
        printLnCString("PARAMETER_BACKCOLOR_G\n");
        printLnUINT8(digifiz_parameters.backc_g.value);
        break;
      case PARAMETER_BACKCOLOR_B:
        printLnCString("PARAMETER_BACKCOLOR_B\n");
        printLnUINT8(digifiz_parameters.backc_b.value);
        break;
      case PARAMETER_RPM_FILTER:
        printLnCString("PARAMETER_RPM_FILTER\n");
        printLnUINT8(digifiz_parameters.rpmFilterK.value);
        break;
      case PARAMETER_SPEED_FILTER:
        printLnCString("PARAMETER_SPEED_FILTER\n");
        printLnUINT8(digifiz_parameters.speedFilterK.value);
        break;
      case PARAMETER_SET_RPM_OPTIONS: //TODO: move to new params
        printLnCString("PARAMETER_GET_RPM_OPTIONS\n");
        // printLnUINT8(digifiz_parameters.rpmOptions_redline_segments.value);
        break;
      case PARAMETER_SET_TEMP_OPTIONS:
        printLnCString("PARAMETER_GET_TEMP_OPTIONS\n");
        // printLnUINT8(digifiz_parameters.tempOptions_alarm_function.value);
        break;
      case PARAMETER_SET_SIGNAL_OPTIONS:
        printLnCString("PARAMETER_GET_SIGNAL_OPTIONS\n");
        // printLnUINT8(digifiz_parameters.signalOptions_use_blink_alt_in.value);
        break;
      case PARAMETER_PULLUP_RESISTANCE_COOLANT:
        printLnCString("PARAMETER_PULLUP_RESISTANCE_COOLANT\n");
        printLnUINT32(digifiz_parameters.coolantThermistorPullUpRes.value);
        break;
      case PARAMETER_PULLUP_RESISTANCE_OIL:
        printLnCString("PARAMETER_PULLUP_RESISTANCE_OIL\n");
        printLnUINT32(digifiz_parameters.oilThermistorPullUpRes.value);
        break;
      case PARAMETER_PULLUP_RESISTANCE_AMB:
        printLnCString("PARAMETER_PULLUP_RESISTANCE_AMB\n");
        printLnUINT32(digifiz_parameters.ambThermistorPullUpRes.value);
        break;
      default:
        break;
    }
  }
  //saveParameters();
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
                //printf("cmd_buffer_par:%s\n",cmd_buffer_par);
                //printf("cmd_buffer_val:%s\n",cmd_buffer_val);
                if (strcmp(cmd_buffer_par,"help")==0)
                {
                    printHelp();
                }
                else if (strcmp(cmd_buffer_par,"about")==0)
                {
                    printAbout();
                }
                else if (strcmp(cmd_buffer_par,"save")==0)
                {
                    saveParameters();
                }
                else if (strcmp(cmd_buffer_par,"adc")==0)
                {
                    printADC();
                }
                else if (strcmp(cmd_buffer_par,"reset_colors")==0)
                {
                    reinit_load_default_color_scheme();
                    compileColorScheme();
                    printLnCString("RESET COLORS OK\n");
                }
                else if (strcmp(cmd_buffer_par,"test_mode")==0)
                {
                    digifiz_parameters.option_testmode_on.value=!digifiz_parameters.option_testmode_on.value;
                }
                else if (strcmp(cmd_buffer_par,"set_c_main")==0)
                {
                    setMainTemplateColor(cmd_buffer_val);
                }
                else if (strcmp(cmd_buffer_par,"set_c_back")==0)
                {
                    setBacklightTemplateColor(cmd_buffer_val);
                }
                else if (strcmp(cmd_buffer_par,"status_json")==0)
                {
                    printStatusJSON();
                }
                else
                {

                    if (is_number(cmd_buffer_par)) {
                        // Convert the parameter string to an integer
                        uint32_t param_num = strtol(cmd_buffer_par, NULL, 10);
                        if (param_num > 255) { // Adjust the upper bound as needed
                            printLnCString("Parameter number out of range:\n");
                            printLnUINT32(param_num);
                            memset(cmd_buffer,0,sizeof(cmd_buffer));
                            return;
                        }
                        parameter_p = param_num;
                    } else {
                        // Attempt to map the parameter name to its enum value
                        if (!getParameterValue(cmd_buffer_par, &parameter_p)) {
                            printLnCString("Unknown parameter name\n");
                            printLnCString(cmd_buffer_par);
                            memset(cmd_buffer,0,sizeof(cmd_buffer));
                            return;
                        }
                    }

                    value_p = 0;                    
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
