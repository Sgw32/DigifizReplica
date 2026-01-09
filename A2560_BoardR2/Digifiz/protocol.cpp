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

const char param_empty[] PROGMEM = "";
const char param_SPEED_COEFF[]        PROGMEM = "SPEED_COEFF";
const char param_COOLANT_THERM_B[]    PROGMEM = "COOLANT_THERM_B";
const char param_OIL_THERM_B[]        PROGMEM = "OIL_THERM_B";
const char param_AIR_THERM_B[]        PROGMEM = "AIR_THERM_B";
const char param_TANK_MIN_R[]         PROGMEM = "TANK_MIN_R";
const char param_TANK_MAX_R[]         PROGMEM = "TANK_MAX_R";
const char param_TAU_COOLANT[]        PROGMEM = "TAU_COOLANT";
const char param_TAU_OIL[]            PROGMEM = "TAU_OIL";
const char param_TAU_AIR[]            PROGMEM = "TAU_AIR";
const char param_TAU_TANK[]           PROGMEM = "TAU_TANK";
const char param_MILEAGE[]            PROGMEM = "MILEAGE";
const char param_DAILY_MILEAGE[]      PROGMEM = "DAILY_MILEAGE";
const char param_AUTO_BRIGHT[]        PROGMEM = "AUTO_BRIGHT";
const char param_BRIGHT_LEVEL[]       PROGMEM = "BRIGHT_LEVEL";
const char param_TANK_CAP[]           PROGMEM = "TANK_CAP";
const char param_MFA_STATE[]          PROGMEM = "MFA_STATE";
const char param_BUZZER_OFF[]         PROGMEM = "BUZZER_OFF";
const char param_MAX_RPM[]             PROGMEM = "MAX_RPM";
const char param_COOLANT_DEF_R[]       PROGMEM = "COOLANT_DEF_R";
const char param_OIL_DEF_R[]           PROGMEM = "OIL_DEF_R";
const char param_AMB_DEF_R[]           PROGMEM = "AMB_DEF_R";
const char param_RPM_COEFF[]           PROGMEM = "RPM_COEFF";
const char param_DOT_OFF[]             PROGMEM = "DOT_OFF";
const char param_BACKLIGHT_ON[]        PROGMEM = "BACKLIGHT_ON";
const char param_M_D_FILTER[]          PROGMEM = "M_D_FILTER";
const char param_COOLANT_MAX_R[]       PROGMEM = "COOLANT_MAX_R";
const char param_COOLANT_MIN_R[]       PROGMEM = "COOLANT_MIN_R";
const char param_RPM_FILTER[]          PROGMEM = "RPM_FILTER";
const char param_SPEED_FILTER[]        PROGMEM = "SPEED_FILTER";
const char param_RPM_FALL_FILTER[]     PROGMEM = "RPM_FALL_FILTER";
const char param_RPM_QUAD_COEFF[]      PROGMEM = "RPM_QUAD_COEFF";
const char param_CMD_MFA_RESET[]        PROGMEM = "MFA_RESET";
const char param_CMD_MFA_MODE[]         PROGMEM = "MFA_MODE";
const char param_CMD_MFA_BLOCK[]        PROGMEM = "MFA_BLOCK";

const char param_MAINCOLOR_R[]          PROGMEM = "MAINCOLOR_R";
const char param_MAINCOLOR_G[]          PROGMEM = "MAINCOLOR_G";
const char param_MAINCOLOR_B[]          PROGMEM = "MAINCOLOR_B";
const char param_BACKCOLOR_R[]          PROGMEM = "BACKCOLOR_R";
const char param_BACKCOLOR_G[]          PROGMEM = "BACKCOLOR_G";
const char param_BACKCOLOR_B[]          PROGMEM = "BACKCOLOR_B";

const char param_SET_FUEL_CALC[]        PROGMEM = "SET_FUEL_CALC";
const char param_SET_RPM_OPTIONS[]      PROGMEM = "SET_RPM_OPTS";
const char param_SET_TEMP_OPTIONS[]     PROGMEM = "SET_TEMP_OPTS";
const char param_SET_SIGNAL_OPTIONS[]   PROGMEM = "SET_SIGNAL_OPTS";

const char param_MAX_RPM_TH[]           PROGMEM = "MAX_RPM_TH";
const char param_MAX_SPEED_TH[]         PROGMEM = "MAX_SPEED_TH";

const char param_UPTIME[]               PROGMEM = "UPTIME";
const char param_READ_ADDITION[]        PROGMEM = "READ_ADD";

const char param_SET_HOUR[]             PROGMEM = "SET_HOUR";
const char param_SET_MINUTE[]           PROGMEM = "SET_MIN";
const char param_RESET_DAILY[]          PROGMEM = "RESET_DAILY";
const char param_RESET_DIGIFIZ[]        PROGMEM = "RESET_ALL";

const char param_GET_ACC_UPTIME[]       PROGMEM = "GET_ACC_UPTIME";
const char param_GET_COOLANT_TEMP[]     PROGMEM = "GET_COOLANT_TEMP";
const char param_GET_OIL_TEMP[]         PROGMEM = "GET_OIL_TEMP";
const char param_GET_AMBIENT_TEMP[]     PROGMEM = "GET_AMBIENT_TEMP";
const char param_GET_FUEL[]             PROGMEM = "GET_FUEL";
const char param_GET_SPEED[]            PROGMEM = "GET_SPEED";
const char param_GET_RPM[]              PROGMEM = "GET_RPM";

const char param_SET_DAY[]              PROGMEM = "SET_DAY";
const char param_SET_MONTH[]            PROGMEM = "SET_MONTH";
const char param_SET_YEAR[]             PROGMEM = "SET_YEAR";

const char param_GET_DAY[]              PROGMEM = "GET_DAY";
const char param_GET_MONTH[]            PROGMEM = "GET_MONTH";
const char param_GET_YEAR[]             PROGMEM = "GET_YEAR";
const char param_GET_HOUR[]             PROGMEM = "GET_HOUR";
const char param_GET_MINUTE[]           PROGMEM = "GET_MIN";

const char param_GET_GPIO[]             PROGMEM = "GET_GPIO";
const char param_MEM_LOCK[]             PROGMEM = "MEM_LOCK";
const char param_MEM_UNLOCK[]           PROGMEM = "MEM_UNLOCK";

const char param_TOGGLE_MILES[]         PROGMEM = "TOGGLE_MILES";
const char param_TOGGLE_GALLONS[]       PROGMEM = "TOGGLE_GAL";
const char param_TOGGLE_FAHRENHEIT[]    PROGMEM = "TOGGLE_F";
const char param_SAVE_PARAMS[]          PROGMEM = "SAVE_PARAMS";
const char param_TOGGLE_TOUCH[]         PROGMEM = "TOUCH_TOGGLE";

const char *const DigifizParamNames[256] PROGMEM = {
    /*   0 */ param_empty,
    /*   1 */ param_SPEED_COEFF,
    /*   2 */ param_COOLANT_THERM_B,
    /*   3 */ param_OIL_THERM_B,
    /*   4 */ param_AIR_THERM_B,
    /*   5 */ param_TANK_MIN_R,
    /*   6 */ param_TANK_MAX_R,
    /*   7 */ param_TAU_COOLANT,
    /*   8 */ param_TAU_OIL,
    /*   9 */ param_TAU_AIR,
    /*  10 */ param_TAU_TANK,
    /*  11 */ param_MILEAGE,
    /*  12 */ param_DAILY_MILEAGE,
    /*  13 */ param_AUTO_BRIGHT,
    /*  14 */ param_BRIGHT_LEVEL,
    /*  15 */ param_TANK_CAP,
    /*  16 */ param_MFA_STATE,
    /*  17 */ param_BUZZER_OFF,
    /*  18 */ param_MAX_RPM,
    /*  19 */ param_COOLANT_DEF_R,
    /*  20 */ param_OIL_DEF_R,
    /*  21 */ param_AMB_DEF_R,
    /*  22 */ param_RPM_COEFF,
    /*  23 */ param_DOT_OFF,
    /*  24 */ param_BACKLIGHT_ON,
    /*  25 */ param_M_D_FILTER,
    /*  26 */ param_COOLANT_MAX_R,
    /*  27 */ param_COOLANT_MIN_R,
    /*  28 */ param_CMD_MFA_RESET,
    /*  29 */ param_CMD_MFA_MODE,
    /*  30 */ param_CMD_MFA_BLOCK,
    /*  31 */ param_MAINCOLOR_R,
    /*  32 */ param_MAINCOLOR_G,
    /*  33 */ param_MAINCOLOR_B,
    /*  34 */ param_BACKCOLOR_R,
    /*  35 */ param_BACKCOLOR_G,
    /*  36 */ param_BACKCOLOR_B,
    /*  37 */ param_RPM_FILTER,
    /*  38 */ param_SPEED_FILTER,
    /*  39 */ param_SET_FUEL_CALC,
    /*  40 */ param_SET_RPM_OPTIONS,
    /*  41 */ param_SET_TEMP_OPTIONS,
    /*  42 */ param_SET_SIGNAL_OPTIONS,
    /*  43 */ param_MAX_RPM_TH,
    /*  44 */ param_MAX_SPEED_TH,

    /*  45 */ param_empty,
    /*  46 */ param_empty,
    /*  47 */ param_empty,
    /*  48 */ param_empty,
    /*  49 */ param_empty,
    /*  50 */ param_empty,
    /*  51 */ param_empty,

    /*  52 */ param_RPM_FALL_FILTER,
    /*  53 */ param_RPM_QUAD_COEFF,

    /*  54 */ param_empty,
    /*  55 */ param_empty,
    /*  56 */ param_empty,
    /*  57 */ param_empty,
    /*  58 */ param_empty,
    /*  59 */ param_empty,
    /*  60 */ param_empty,
    /*  61 */ param_empty,
    /*  62 */ param_empty,
    /*  63 */ param_empty,
    /*  64 */ param_empty,
    /*  65 */ param_empty,
    /*  66 */ param_empty,
    /*  67 */ param_empty,
    /*  68 */ param_empty,
    /*  69 */ param_empty,
    /*  70 */ param_empty,
    /*  71 */ param_empty,
    /*  72 */ param_empty,
    /*  73 */ param_empty,
    /*  74 */ param_empty,
    /*  75 */ param_empty,
    /*  76 */ param_empty,
    /*  77 */ param_empty,
    /*  78 */ param_empty,
    /*  79 */ param_empty,
    /*  80 */ param_empty,
    /*  81 */ param_empty,
    /*  82 */ param_empty,
    /*  83 */ param_empty,
    /*  84 */ param_empty,
    /*  85 */ param_empty,
    /*  86 */ param_empty,
    /*  87 */ param_empty,
    /*  88 */ param_empty,
    /*  89 */ param_empty,
    /*  90 */ param_empty,
    /*  91 */ param_empty,
    /*  92 */ param_empty,
    /*  93 */ param_empty,
    /*  94 */ param_empty,
    /*  95 */ param_empty,
    /*  96 */ param_empty,
    /*  97 */ param_empty,
    /*  98 */ param_empty,
    /*  99 */ param_empty,
    /* 100 */ param_empty,
    /* 101 */ param_empty,
    /* 102 */ param_empty,
    /* 103 */ param_empty,
    /* 104 */ param_empty,
    /* 105 */ param_empty,
    /* 106 */ param_empty,
    /* 107 */ param_empty,
    /* 108 */ param_empty,
    /* 109 */ param_empty,
    /* 110 */ param_empty,
    /* 111 */ param_empty,
    /* 112 */ param_empty,
    /* 113 */ param_empty,
    /* 114 */ param_empty,
    /* 115 */ param_empty,
    /* 116 */ param_empty,
    /* 117 */ param_empty,
    /* 118 */ param_empty,
    /* 119 */ param_empty,
    /* 120 */ param_empty,
    /* 121 */ param_empty,
    /* 122 */ param_empty,

    /* 123 */ param_UPTIME,
    /* 124 */ param_empty,
    /* 125 */ param_empty,
    /* 126 */ param_empty,
    /* 127 */ param_empty,
    /* 128 */ param_READ_ADDITION,

    /* 129 */ param_empty,
    /* 130 */ param_empty,
    /* 131 */ param_empty,
    /* 132 */ param_empty,
    /* 133 */ param_empty,
    /* 134 */ param_empty,
    /* 135 */ param_empty,
    /* 136 */ param_empty,
    /* 137 */ param_empty,
    /* 138 */ param_empty,
    /* 139 */ param_empty,
    /* 140 */ param_empty,
    /* 141 */ param_empty,
    /* 142 */ param_empty,
    /* 143 */ param_empty,
    /* 144 */ param_empty,
    /* 145 */ param_empty,
    /* 146 */ param_empty,
    /* 147 */ param_empty,
    /* 148 */ param_empty,
    /* 149 */ param_empty,
    /* 150 */ param_empty,
    /* 151 */ param_empty,
    /* 152 */ param_empty,
    /* 153 */ param_empty,
    /* 154 */ param_empty,
    /* 155 */ param_empty,
    /* 156 */ param_empty,
    /* 157 */ param_empty,
    /* 158 */ param_empty,
    /* 159 */ param_empty,
    /* 160 */ param_empty,
    /* 161 */ param_empty,
    /* 162 */ param_empty,
    /* 163 */ param_empty,
    /* 164 */ param_empty,
    /* 165 */ param_empty,
    /* 166 */ param_empty,
    /* 167 */ param_empty,
    /* 168 */ param_empty,
    /* 169 */ param_empty,
    /* 170 */ param_empty,
    /* 171 */ param_empty,
    /* 172 */ param_empty,
    /* 173 */ param_empty,
    /* 174 */ param_empty,
    /* 175 */ param_empty,
    /* 176 */ param_empty,
    /* 177 */ param_empty,
    /* 178 */ param_empty,
    /* 179 */ param_empty,
    /* 180 */ param_empty,
    /* 181 */ param_empty,
    /* 182 */ param_empty,
    /* 183 */ param_empty,
    /* 184 */ param_empty,
    /* 185 */ param_empty,
    /* 186 */ param_empty,
    /* 187 */ param_empty,
    /* 188 */ param_empty,
    /* 189 */ param_empty,
    /* 190 */ param_empty,
    /* 191 */ param_empty,
    /* 192 */ param_empty,
    /* 193 */ param_empty,
    /* 194 */ param_empty,
    /* 195 */ param_empty,
    /* 196 */ param_empty,
    /* 197 */ param_empty,
    /* 198 */ param_empty,
    /* 199 */ param_empty,

    /* 200 */ param_empty,
    /* 201 */ param_empty,
    /* 202 */ param_empty,
    /* 203 */ param_empty,
    /* 204 */ param_empty,
    /* 205 */ param_empty,
    /* 206 */ param_empty,
    /* 207 */ param_empty,
    /* 208 */ param_empty,
    /* 209 */ param_empty,
    /* 210 */ param_empty,
    /* 211 */ param_empty,
    /* 212 */ param_empty,
    /* 213 */ param_empty,
    /* 214 */ param_empty,
    /* 215 */ param_empty,
    /* 216 */ param_empty,
    /* 217 */ param_empty,
    /* 218 */ param_empty,
    /* 219 */ param_empty,
    /* 220 */ param_empty,
    /* 221 */ param_empty,
    /* 222 */ param_empty,
    /* 223 */ param_empty,
    /* 224 */ param_empty,
    /* 225 */ param_empty,
    /* 226 */ param_empty,
    /* 227 */ param_empty,
    /* 228 */ param_empty,
    /* 229 */ param_TOGGLE_TOUCH,
    /* 230 */ param_SAVE_PARAMS,
    /* 231 */ param_TOGGLE_FAHRENHEIT,
    /* 232 */ param_TOGGLE_GALLONS,
    /* 233 */ param_TOGGLE_MILES,
    /* 234 */ param_MEM_UNLOCK,
    /* 235 */ param_MEM_LOCK,
    /* 236 */ param_GET_GPIO,
    /* 237 */ param_GET_MINUTE,
    /* 238 */ param_GET_HOUR,
    /* 239 */ param_GET_YEAR,
    /* 240 */ param_GET_MONTH,
    /* 241 */ param_GET_DAY,
    /* 242 */ param_SET_YEAR,
    /* 243 */ param_SET_MONTH,
    /* 244 */ param_SET_DAY,
    /* 245 */ param_GET_RPM,
    /* 246 */ param_GET_SPEED,
    /* 247 */ param_GET_FUEL,
    /* 248 */ param_GET_AMBIENT_TEMP,
    /* 249 */ param_GET_OIL_TEMP,
    /* 250 */ param_GET_COOLANT_TEMP,
    /* 251 */ param_GET_ACC_UPTIME,
    /* 252 */ param_RESET_DIGIFIZ,
    /* 253 */ param_RESET_DAILY,
    /* 254 */ param_SET_MINUTE,
    /* 255 */ param_SET_HOUR,
};



static void printParameterName(int parameter)
{
#ifdef USE_BTSERIAL
  char buffer[30];
  BTserial.print("PARAMETER_");
  strcpy_P(buffer, (char *)pgm_read_ptr(&(DigifizParamNames[(uint8_t)parameter])));
  BTserial.println(buffer);
  #endif
}
 

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

static void processWrite(int parameter,long value)
{
  int par = parameter;
    switch(par)
    {
      case PARAMETER_RPMCOEFFICIENT:
        digifiz_parameters.rpmCoefficient.value = value;
        break;
      case PARAMETER_RPM_QUADRATIC_COEFFICIENT:
        //digifiz_parameters.rpmQuadraticCoefficient.value = value;
        break;
      case PARAMETER_SPEEDCOEEFICIENT:
        digifiz_parameters.speedCoefficient.value = value;
        break;
      case PARAMETER_COOLANTTHERMISTORB:
        digifiz_parameters.coolantThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_OILTHERMISTORB:
        digifiz_parameters.oilThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_AIRTHERMISTORB:
        digifiz_parameters.airThermistorB.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_TANKMINRESISTANCE:
        digifiz_parameters.tankMinResistance.value = value;
        updateADCSettings();
        break;  
      case PARAMETER_TANKMAXRESISTANCE:
        digifiz_parameters.tankMaxResistance.value = value;
        updateADCSettings();
        break; 
      case PARAMETER_TAU_COOLANT:
        digifiz_parameters.tauCoolant.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_OIL:
        digifiz_parameters.tauOil.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_AIR:
        digifiz_parameters.tauAir.value = value;
        updateADCSettings();
        break;
      case PARAMETER_TAU_TANK:
        digifiz_parameters.tauTank.value = value;
        break;
      case PARAMETER_MILEAGE:
        digifiz_parameters.mileage.value = (uint32_t)value*3600;
        break;
      case PARAMETER_DAILY_MILEAGE:
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
        digifiz_parameters.autoBrightness.value = value;
        break;
      case PARAMETER_BRIGHTNESS_LEVEL:
        digifiz_parameters.brightnessLevel.value = value;
        break;
      case PARAMETER_TANK_CAPACITY:
        digifiz_parameters.tankCapacity.value = value;
        updateADCSettings();
        break;
      case PARAMETER_MFA_STATE:
        digifiz_parameters.mfaState.value = value;
        break;
      case PARAMETER_BUZZER_OFF:
        digifiz_parameters.buzzerOff.value = value;
        break;
      case PARAMETER_MAX_RPM:
        digifiz_parameters.maxRPM.value = value;
        break;
      case PARAMETER_NORMAL_RESISTANCE_COOLANT:
        digifiz_parameters.coolantThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_NORMAL_RESISTANCE_OIL:
        digifiz_parameters.oilThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_NORMAL_RESISTANCE_AMB:
        digifiz_parameters.ambThermistorDefRes.value = value;
        updateADCSettings();
        break;
      case PARAMETER_DOT_OFF:
        digifiz_parameters.displayDot.value = value;
        break;
      case PARAMETER_BACKLIGHT_ON:
        digifiz_parameters.backlight_on.value = value;
        break;
      case PARAMETER_M_D_FILTER:
        //digifiz_parameters.medianDispFilterThreshold.value = value;
        break;
      case PARAMETER_COOLANT_MAX_R:
        digifiz_parameters.coolantMaxResistance.value = value;
        updateADCSettings();
        break;
      case PARAMETER_COOLANT_MIN_R:
        digifiz_parameters.coolantMinResistance.value = value;
        updateADCSettings();
        break;
      case PARAMETER_COMMAND_MFA_RESET:
	      pressMFAReset();
        break;
      case PARAMETER_COMMAND_MFA_MODE:
        pressMFAMode();
        break;
      case PARAMETER_COMMAND_MFA_BLOCK:
        pressMFABlock();
        break;
      case PARAMETER_RPM_FILTER:
        digifiz_parameters.rpmFilterK.value = value;
        break;
      case PARAMETER_RPM_FALLING_FILTER:
        digifiz_parameters.rpmFallingFilterK.value = value;
        break;
      case PARAMETER_SPEED_FILTER:
        digifiz_parameters.speedFilterK.value = value;
        break;
      case PARAMETER_SET_FUEL_CALC_FUNCTION:
        digifiz_parameters.option_linear_fuel.value = value&1;
        break;
      case PARAMETER_SET_RPM_OPTIONS:
        digifiz_parameters.rpmOptions_redline_segments.value = value & 0x1F;
        break;
      case PARAMETER_SET_TEMP_OPTIONS:
        digifiz_parameters.tempOptions_red_segments.value = value & 0x3;
        digifiz_parameters.tempOptions_sensor_connected_ind.value = (value >> 2) & 0x1;
        digifiz_parameters.tempOptions_alarm_function.value = (value >> 3) & 0x1;
        break;
      case PARAMETER_SET_SIGNAL_OPTIONS:
        digifiz_parameters.signalOptions_use_blink_alt_in.value = value & 0x1;
        digifiz_parameters.signalOptions_enable_touch_sensor.value = (value >> 1) & 0x1;
        //digifiz_parameters.signalOptions_invert_light_input.value = (value >> 2) & 0x1;
        break;
      case PARAMETER_MAX_RPM_THRESHOLD:
        digifiz_parameters.rpmMaxThreshold.value = value;
        break;
      case PARAMETER_MAX_SPEED_THRESHOLD:
        digifiz_parameters.speedMaxThreshold.value = value;
        break;
      case PARAMETER_UPTIME:
        digifiz_parameters.uptime.value = value;
        break;  
      default:
        break;
    }
}

static void processRead(int parameter)
{
  int par = parameter - PARAMETER_READ_ADDITION;
    
  //reads
  switch(par)
  {
    case PARAMETER_RPMCOEFFICIENT:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.rpmCoefficient.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.rpmCoefficient.value);
      #endif
      break;
    case PARAMETER_RPM_QUADRATIC_COEFFICIENT:
    #ifdef USE_BTSERIAL        
      BTserial.println(0);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(0);
      #endif
      break;
    case PARAMETER_SPEEDCOEEFICIENT:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.speedCoefficient.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.speedCoefficient.value);
      #endif
      break;
    case PARAMETER_COOLANTTHERMISTORB:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.coolantThermistorB.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.coolantThermistorB.value);
      #endif
      break;  
    case PARAMETER_OILTHERMISTORB:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.oilThermistorB.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.oilThermistorB.value);
      #endif
      break;  
    case PARAMETER_AIRTHERMISTORB:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.airThermistorB.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.airThermistorB.value);
      #endif
      break;  
    case PARAMETER_TANKMINRESISTANCE:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.tankMinResistance.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.tankMinResistance.value);
      #endif
      break;  
    case PARAMETER_TANKMAXRESISTANCE:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.tankMaxResistance.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.tankMaxResistance.value);
      #endif
      break; 
    case PARAMETER_TAU_COOLANT:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.tauCoolant.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.tauCoolant.value);
      #endif
      break;
    case PARAMETER_TAU_OIL:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.tauOil.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.tauOil.value);
      #endif
      break;
    case PARAMETER_TAU_AIR:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.tauAir.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.tauAir.value);
      #endif
      break;
    case PARAMETER_TAU_TANK:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.tauTank.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.tauTank.value);
      #endif
      break;
    case PARAMETER_MILEAGE:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.mileage.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.mileage.value);
      #endif
      break;
    case PARAMETER_DAILY_MILEAGE:
    #ifdef USE_BTSERIAL
      BTserial.println(dailyMileage(digifiz_parameters.mfaBlock.value));
      #endif
      #ifdef USE_UIOD
      UIODserial.println(dailyMileage(digifiz_parameters.mfaBlock.value));
      #endif
      break;
    case PARAMETER_AUTO_BRIGHTNESS:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.autoBrightness.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.autoBrightness.value);
      #endif
      break;
    case PARAMETER_BRIGHTNESS_LEVEL:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.brightnessLevel.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.brightnessLevel.value);
      #endif
      break;
    case PARAMETER_TANK_CAPACITY:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.tankCapacity.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.tankCapacity.value);
      #endif
      break;
    case PARAMETER_MFA_STATE:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.mfaState.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.mfaState.value);
      #endif
      break;
    case PARAMETER_BUZZER_OFF:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.buzzerOff.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.buzzerOff.value);
      #endif
      break;
    case PARAMETER_MAX_RPM:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.maxRPM.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.maxRPM.value);
      #endif
      break;
    case PARAMETER_BACKLIGHT_ON:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.backlight_on.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.backlight_on.value);
      #endif
      break;
    case PARAMETER_M_D_FILTER:
    #ifdef USE_BTSERIAL
      BTserial.println(65535);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(65535);
      #endif
      break;
    case PARAMETER_COOLANT_MAX_R:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.coolantMaxResistance.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.coolantMaxResistance.value);
      #endif
      break;
    case PARAMETER_COOLANT_MIN_R:
    #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.coolantMinResistance.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.coolantMinResistance.value);
      #endif
      break;
    case PARAMETER_COMMAND_MFA_RESET:
      pressMFAReset();
      break;
    case PARAMETER_COMMAND_MFA_MODE:
      pressMFAMode();
      break;
    case PARAMETER_COMMAND_MFA_BLOCK:
      pressMFABlock();
      break;
    case PARAMETER_RPM_FILTER:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.rpmFilterK.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.rpmFilterK.value);
      #endif
      break;
    case PARAMETER_RPM_FALLING_FILTER:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.rpmFallingFilterK.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.rpmFallingFilterK.value);
      #endif
      break;
    case PARAMETER_SPEED_FILTER:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.speedFilterK.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.speedFilterK.value);
      #endif
      break;
    case PARAMETER_SET_FUEL_CALC_FUNCTION:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.option_linear_fuel.value );
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.option_linear_fuel.value);
      #endif
      break;
    case PARAMETER_SET_RPM_OPTIONS:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.rpmOptions_redline_segments.value & 0x1F);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.rpmOptions_redline_segments.value & 0x1F);
      #endif
      break;
    case PARAMETER_SET_TEMP_OPTIONS:
      #ifdef USE_BTSERIAL
      BTserial.println((digifiz_parameters.tempOptions_red_segments.value & 0x3) | ((digifiz_parameters.tempOptions_sensor_connected_ind.value & 0x1)<<2) | ((digifiz_parameters.tempOptions_alarm_function.value &0x1)<<3));
      #endif
      #ifdef USE_UIOD
      UIODserial.println((digifiz_parameters.tempOptions_red_segments.value & 0x3) | ((digifiz_parameters.tempOptions_sensor_connected_ind.value & 0x1)<<2) | ((digifiz_parameters.tempOptions_alarm_function.value &0x1)<<3));
      #endif
      break;
    case PARAMETER_SET_SIGNAL_OPTIONS:
      #ifdef USE_BTSERIAL
      BTserial.println((digifiz_parameters.signalOptions_use_blink_alt_in.value & 0x1) | ((digifiz_parameters.signalOptions_enable_touch_sensor.value & 0x1)<<1));
      #endif
      #ifdef USE_UIOD
      UIODserial.println((digifiz_parameters.signalOptions_use_blink_alt_in.value & 0x1) | ((digifiz_parameters.signalOptions_enable_touch_sensor.value & 0x1)<<1));
      #endif
      break;
    case PARAMETER_MAX_RPM_THRESHOLD:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.rpmMaxThreshold.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.rpmMaxThreshold.value);
      #endif
      break;
    case PARAMETER_MAX_SPEED_THRESHOLD:
      #ifdef USE_BTSERIAL
      BTserial.println(digifiz_parameters.speedMaxThreshold.value);
      #endif
      #ifdef USE_UIOD
      UIODserial.println(digifiz_parameters.speedMaxThreshold.value);
      #endif
      break;
    case PARAMETER_UPTIME:
      #ifdef USE_BTSERIAL        
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

static void processCommands(int par,long value)
{
    DateTime tme = myRTC.now();
    //commands
    if (par==PARAMETER_SET_HOUR)
    {
      DateTime newTime1 = DateTime(tme.year(), tme.month(), tme.day(), constrain(value,0,24),tme.minute(),0);
      myRTC.adjust(newTime1);
    }
    else if (par==PARAMETER_TOGGLE_MILES)
    {
      if (!(digifiz_parameters.option_miles.value))
        digifiz_parameters.option_miles.value = 1;
      else
        digifiz_parameters.option_miles.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_GALLONS)
    {
      if (!(digifiz_parameters.option_gallons.value))
        digifiz_parameters.option_gallons.value = 1;
      else
        digifiz_parameters.option_gallons.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_FAHRENHEIT)
    {
      if (!(digifiz_parameters.option_fahrenheit.value))
        digifiz_parameters.option_fahrenheit.value = 1;
      else
        digifiz_parameters.option_fahrenheit.value = 0;
    }
    else if (par==PARAMETER_TOGGLE_TOUCH_SENSOR)
    {
      if (!(digifiz_parameters.signalOptions_enable_touch_sensor.value))
        digifiz_parameters.signalOptions_enable_touch_sensor.value = 1;
      else
        digifiz_parameters.signalOptions_enable_touch_sensor.value = 0;
    }
    else if (par==PARAMETER_SAVE_PARAMS)
    {
      saveParameters();
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
    switch(par)
    {
      case PARAMETER_GET_ACCUMULATED_UPTIME:
        #ifdef USE_BTSERIAL
        BTserial.println(digifiz_parameters.uptime.value);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(digifiz_parameters.uptime.value);
        #endif
        break;
      case PARAMETER_GET_COOLANT_TEMPERATURE:
        #ifdef USE_BTSERIAL
        BTserial.println(getCoolantTemperature());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getCoolantTemperature());
        #endif
        break;
      case PARAMETER_GET_AMBIENT_TEMPERATURE:
        #ifdef USE_BTSERIAL
        BTserial.println(getAmbientTemperature());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getAmbientTemperature());
        #endif
        break;
      case PARAMETER_GET_OIL_TEMPERATURE:
      #ifdef USE_BTSERIAL
        BTserial.println(getOilTemperature());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getOilTemperature());
        #endif
        break;
      case PARAMETER_GET_FUEL_IN_TANK:
      #ifdef USE_BTSERIAL
        BTserial.println(getLitresInTank());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(getLitresInTank());
        #endif
        break;
      case PARAMETER_GET_SPEED:
      #ifdef USE_BTSERIAL
        BTserial.println(spd_m_speedometer);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(spd_m_speedometer);
        #endif
        break;
      case PARAMETER_GET_RPM:
      #ifdef USE_BTSERIAL
        BTserial.println(averageRPM);
        #endif
        #ifdef USE_UIOD
        UIODserial.println(averageRPM);
        #endif
        break;
      case PARAMETER_GET_DAY:
      #ifdef USE_BTSERIAL
        BTserial.println(myRTC.now().day());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().day());
        #endif
        break;
      case PARAMETER_GET_MONTH:
      #ifdef USE_BTSERIAL
        BTserial.println(myRTC.now().month());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().month());
        #endif
        break;
      case PARAMETER_GET_YEAR:
      #ifdef USE_BTSERIAL
        BTserial.println(myRTC.now().year());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().year());
        #endif
        break;
      case PARAMETER_GET_HOUR:
      #ifdef USE_BTSERIAL
        BTserial.println(myRTC.now().hour());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().hour());
        #endif
        break;
      case PARAMETER_GET_MINUTE:
      #ifdef USE_BTSERIAL
        BTserial.println(myRTC.now().minute());
        #endif
        #ifdef USE_UIOD
        UIODserial.println(myRTC.now().minute());
        #endif
        break;
      case PARAMETER_GET_GPIO_PINS:
        processGPIOPinsValue(value);
        break;
      case PARAMETER_MEMORY_LOCK:
        if (value==123)
           protocol_locked = 1;
        break;        
      case PARAMETER_MEMORY_UNLOCK:
        if (value==123)
           protocol_locked = 0;
        break;        

      default:
        break;
    }
}

static void processData(int parameter,long value)
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
    processWrite(par, value);
  }
  else
  {
    processCommands(par,value);
    processRead(par);
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
            printParameterName(parameter_p);
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
