#include "adc.h"
#include "setup.h"

const float R1_Coolant = COOLANT_R_AT_NORMAL_T; //for Coolant

#ifndef NEW_REVISION
const float R2_Ambient = 10000.0f; //for Coolant
#else
const float R2_Ambient = 1000.0f; //for Coolant
#endif

#ifdef OIL_RES_10000
const float R2_Oil = 220;
#else
const float R2_Oil = 220;
#endif

const float R1_Oil = OIL_R_AT_NORMAL_T; //for Coolant
const float R1_Ambient = AMBIENT_R_AT_NORMAL_T; //for Coolant

float logR2, R2, coolantT, oilT, airT;
float coolantB = 4000;
float oilB = 4000;
float airB = 4000;
float V0;
float gasolineLevel,gasolineLevelFiltered,gasolineLevelFiltered05hour;

float tauCoolant, tauGasoline, tauAir, tauOil,tauGasolineConsumption;
float consumptionLevel;
uint8_t oil03Pin = A1; // Oil pressure 0.3
uint8_t oil08Pin = A2; // Oil pressure 0.8
uint8_t lightSensorPin = A9; // Light sensor
uint8_t coolantPin = A10; // Coolant temp sensor
uint8_t gasolinePin = A11; //Gasoline sensor
uint8_t oilPin = A14; //Gasoline sensor
uint8_t airPin = A15; //Gasoline sensor
uint8_t pressurePin = A5;

uint8_t tankCapacity = 60;
uint16_t lightLevel;

extern float averageRPM;
//#define TAU_GASOLINE 0.02f
//#define TAU_COOLANT 0.02f

extern float spd_m_speedometer;

#define TAU 0.01

uint32_t consumptionCounter;

extern digifiz_pars digifiz_parameters;

void initADC()
{
    //pinMode(A0, INPUT); //GURTANL.K
    pinMode(oil03Pin, INPUT); //OEL 0.3 BAR - oil pressure 0.3 bar
    pinMode(oil08Pin, INPUT); //OEL 1.8 BAR - oil pressure 1.8 bar
    pinMode(lightSensorPin, INPUT); //Light sensor to adjust brightness
    pinMode(coolantPin, INPUT); //Coolant temperature sensor
    pinMode(gasolinePin, INPUT); //Gasoline tank level sensor
    pinMode(oilPin, INPUT); //Coolant temperature sensor
    pinMode(airPin, INPUT); //Gasoline tank level sensor
    consumptionCounter = millis();
    coolantT = oilT = airT = 0.0;
    lightLevel = 0;
    coolantB = digifiz_parameters.coolantThermistorB;
    oilB = digifiz_parameters.oilThermistorB;
    airB = digifiz_parameters.airThermistorB;
    tauCoolant = (float)digifiz_parameters.tauCoolant*TAU;
    tauOil = (float)digifiz_parameters.tauOil*TAU*0.1;
    tauAir = (float)digifiz_parameters.tauAir*TAU*0.1;
    tauGasoline = (float)digifiz_parameters.tauTank*TAU*0.01;
    tauGasolineConsumption = (float)digifiz_parameters.tauTank*TAU*0.01;
    tankCapacity = digifiz_parameters.tankCapacity;
    
    //Init values:
    processFirstCoolantTemperature();
    processFirstOilTemperature();
    processFirstGasLevel();
    processFirstAmbientTemperature();
}

uint16_t getRawBrightnessLevel()
{
  return lightLevel;
}

uint8_t getBrightnessLevel()
{
  //#ifndef YELLOW_GREEN_LED
  return constrain(lightLevel/4,2,15); //0..0.3V -> 0..80 (~255)
  //#else
  //return constrain(lightLevel/4+2,2,15); //0..0.3V -> 0..80 (~255)
  //#endif
}

void processBrightnessLevel()
{
   uint16_t lData = analogRead(lightSensorPin);
   lData += analogRead(lightSensorPin);
   lData += analogRead(lightSensorPin);
   lData += analogRead(lightSensorPin);
   lData += analogRead(lightSensorPin);
   lData += analogRead(lightSensorPin);
   lData += analogRead(lightSensorPin);
   if (lData>800)
    return; //I do not believe you.
   
   lightLevel = lData>>3;
}

//RAW values 0..1024
uint16_t getRawCoolantTemperature()
{
    return analogRead(coolantPin);
}

uint16_t getRawOilTemperature()
{
    return analogRead(oilPin);
}

uint16_t getRawGasLevel()
{
    return analogRead(gasolinePin);
}

uint16_t getRawAmbientTemperature()
{
    return analogRead(airPin);
}

uint16_t getRawLightLevel()
{
    return analogRead(lightSensorPin);
}

//Data values
float getCoolantTemperature()
{
    if (coolantT<-50.0f)
        return -999.9f;
    else if (coolantT>200.0)
        return -999.9f;
    else
        return coolantT;
}

float getOilTemperature()
{
    V0 = (float)analogRead(oilPin);
    R2 = R2_Oil * V0 / (1023.0f - V0); //
    float tempT = 1.0f/(log(R2/R1_Oil)/coolantB+1.0f/(25.0f+273.15f))-273.15f;
    if (tempT<-50.0f)
        return -999.9f;
    else if (tempT>200.0)
        return -999.9f;
    else
        return oilT +=0.1*(tempT-oilT);
}


float getOilTemperatureFahrenheit()
{
   float res = getOilTemperature();
  return (res*1.8f)+32.0f;
}

void processCoolantTemperature()
{
    V0 = (float)analogRead(coolantPin);
    R2 = 220.0f * V0 / (1023.0f - V0); //
    float temp1 = (log(R2/R1_Coolant)/coolantB);
    temp1 += 1/(25.0f+273.15f);
    coolantT += tauCoolant*(1.0f/temp1 - 273.15f - coolantT);
}

void processOilTemperature()
{
    V0 = (float)analogRead(oilPin);
    R2 = R2_Oil * V0 / (1023.0f - V0); //
    float temp1 = (log(R2/R1_Oil)/oilB);
    temp1 += 1/(25.0f+273.15f);
    oilT += tauOil*(1.0f/temp1 - 273.15f - oilT);
}

float getIntakeVoltage()
{
    float intp = (float)analogRead(pressurePin);
    return intp/1023.0f*5.0f;
}

float getIntakePressure()
{
    float intp;
    for (int i=0;i!=100;i++)
     intp += (float)analogRead(pressurePin);
    intp/=100;

#ifdef FUEL_CONSUMPTION_TESTMODE
   intp = 512;
#endif
   
    return 84749.0f-20152.0f*intp/1023.0f*5.0f;
}

float getCurrentIntakeFuelConsumption()
{
    //http://sergeyk.kiev.ua/avto/car_fuel_calc/
    //https://github.com/oesmith/obdgpslogger/blob/master/doc/mpg-calculation
    float kP = constrain(getIntakePressure()/1000.0f,0.0f,120.0f); // pressure in kPa 
    float lp100km = 0.0f;
    if (kP>0)
    {
      float intakeT = constrain(getAmbientTemperature(),-20.0f,30.0f)+273.0f; //intakeT in K
      //float intakeT = 273.0f;
      const float Rtd = 8.314f; //thermodynamic constant
      const float MM = 28.97f; //air molecular mass
      const float engineV = 1.6f; //engine displacement
      const float volEfficiency = 0.65; //65% volumetric efficiency
      float corrPressure = averageRPM*kP/intakeT/2.0f; //corrected reduced pressure
      float maf = (corrPressure/60.0f)*volEfficiency*MM*engineV/Rtd;//mass fuel intake g/second
      const float gasolineDensity = 0.76; //g/cm3
      float lps = maf/gasolineDensity/1000.0f/14.7f;
      float lph = lps*3600.0f; //liters per hour
      //lp100km = lph;
      if (spd_m_speedometer>10.0f)
        lp100km = lph*100.0f/spd_m_speedometer;
      else
          lp100km = lph;
    }
    return constrain(lp100km,0,100.0f);
    //return constrain(kP,0,100.0f);
}

void processGasLevel()
{
    V0 = (float)analogRead(gasolinePin);
    R2 = constrain(220 * V0 / (1023.0f - V0),digifiz_parameters.tankMinResistance,digifiz_parameters.tankMaxResistance); // 330 Ohm in series with fuel sensor
    gasolineLevel += tauGasoline*((1.0f-((float)R2-digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                        digifiz_parameters.tankMinResistance))-gasolineLevel); //percents
    gasolineLevelFiltered += tauGasolineConsumption*(((float)R2-
              digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                                                digifiz_parameters.tankMinResistance)-gasolineLevelFiltered); //percents
    if ((millis()-consumptionCounter)>1800000)
    {
      //half hour
      consumptionCounter = millis();
      consumptionLevel = (gasolineLevelFiltered05hour-gasolineLevelFiltered)*2.0f;
      gasolineLevelFiltered05hour = gasolineLevelFiltered;
    }
}

void processAmbientTemperature()
{
    V0 = (float)analogRead(airPin);
    R2 = R2_Ambient * V0 / (1023.0f - V0); 
    float temp1 = (log(R2/R1_Ambient)/airB);
    temp1 += 1/(25.0f+273.15f);
    airT += tauAir*(1.0f/temp1 - 273.15f - airT);
}

void processFirstCoolantTemperature() //to prevent filtering from zero value
{
    for (int i=0;i!=100;i++)
      V0 += (float)analogRead(coolantPin);
    V0/=100;
    R2 = 220.0f * V0 / (1023.0f - V0); //
    float temp1 = (log(R2/R1_Coolant)/coolantB);
    temp1 += 1/(25.0f+273.15f);
    coolantT = (1.0f/temp1 - 273.15f);
}

void processFirstOilTemperature()
{
    for (int i=0;i!=100;i++)
      V0 += (float)analogRead(oilPin);
    V0/=100;
    R2 = R2_Oil * V0 / (1023.0f - V0); //
    float temp1 = (log(R2/R1_Oil)/oilB);
    temp1 += 1/(25.0f+273.15f);
    oilT = (1.0f/temp1 - 273.15f );
}

void processFirstGasLevel()
{
    for (int i=0;i!=300;i++)
      V0 += (float)analogRead(gasolinePin);
    V0/=300;
    R2 = constrain(220 * V0 / (1023.0f - V0),digifiz_parameters.tankMinResistance,digifiz_parameters.tankMaxResistance); // 330 Ohm in series with fuel sensor
    //35 = full
    //265 = empty
    gasolineLevel = 1.0f - (((float)R2-(float)digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                        digifiz_parameters.tankMinResistance)); //percents
    gasolineLevelFiltered = 1.0f - (((float)R2-(float)digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                        digifiz_parameters.tankMinResistance)); //percents
}

void processFirstAmbientTemperature()
{
    for (int i=0;i!=100;i++)
      V0 += (float)analogRead(airPin);
    V0/=100;
    R2 = R2_Ambient * V0 / (1023.0f - V0); 
    float temp1 = (log(R2/R1_Ambient)/airB);
    temp1 += 1/(25.0f+273.15f);
    airT = temp1;
}

float getGasLevel()
{
    V0 = (float)analogRead(gasolinePin);
    R2 = constrain(220 * V0 / (1023.0f - V0),35,265); // 330 Ohm in series with fuel sensor
    gasolineLevel += tauGasoline*(((float)R2-
              digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                                                digifiz_parameters.tankMinResistance)-gasolineLevel); //percents
    gasolineLevelFiltered += tauGasolineConsumption*(((float)R2-
              digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                                                digifiz_parameters.tankMinResistance)-gasolineLevelFiltered); //percents
    return gasolineLevel;
}

uint8_t getLitresInTank() //0..99
{
    return constrain(gasolineLevel*(float)tankCapacity,0,99); //where 99 of course means error
}

uint8_t getGallonsInTank() //0..99
{
    return constrain(gasolineLevel*(float)tankCapacity*0.264172f,0,99); //where 99 of course means error 
}

uint8_t getDisplayedCoolantTemp()  //0..14, 0..16
{
#ifdef AUDI_DISPLAY
    //16 LEDs
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance)/
            (digifiz_parameters.coolantMaxResistance - digifiz_parameters.coolantMinResistance)*16.0f),0,16);
#endif
#ifdef AUDI_RED_DISPLAY
    //17 LEDs
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance)/
            (digifiz_parameters.coolantMaxResistance - digifiz_parameters.coolantMinResistance)*17.0f),0,17);

#endif

#if !defined(AUDI_RED_DISPLAY) && !defined(AUDI_DISPLAY)
    //14 LEDs
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance)/
            (digifiz_parameters.coolantMaxResistance - digifiz_parameters.coolantMinResistance)*14.0f),0,14); 
 
#endif            
}

uint8_t getDisplayedCoolantTempOrig()  //0..14
{
    //20 LCD segments
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance)/
            (digifiz_parameters.coolantMaxResistance - digifiz_parameters.coolantMinResistance)*20.0f),0,20); 
}


float getAmbientTemperature()
{
    V0 = (float)analogRead(airPin);
    R2 = R2_Ambient * V0 / (1023.0f - V0); //
    float tempT = 1.0f/(log(R2/R1_Ambient)/coolantB+1.0f/(25.0f+273.15f))-273.15f;
    if (tempT<-50.0f)
        return -999.9f;
    else if (tempT>200.0)
        return -999.9f;
    else
    {
        airT +=0.1*(tempT-airT);
        return airT;
    }
    return -999.9f;
}

float getAmbientTemperatureFahrenheit()
{
  float res = getAmbientTemperature();
  return (res*1.8f)+32.0f;
}

float getFuelConsumption()
{
  return constrain(consumptionLevel,0,1.0f);
}
