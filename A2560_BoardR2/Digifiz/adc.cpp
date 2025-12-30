#include "adc.h"
#include "setup.h"
#include <math.h> // for isnan/isfinite

float R1_Coolant = COOLANT_R_AT_NORMAL_T; //for Coolant

#ifndef NEW_REVISION
float R2_Ambient = 10000.0f; //for Coolant
#else
float R2_Ambient = 1000.0f; //for Coolant
#endif

#ifdef OIL_RES_10000
float R2_Oil = 220;
#else
float R2_Oil = 220;
#endif

float R1_Oil = OIL_R_AT_NORMAL_T; //for Coolant
float R1_Ambient = AMBIENT_R_AT_NORMAL_T; //for Coolant

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
uint8_t oilPin = A14; //Oil temp sensor
uint8_t airPin = A15; //Air temp sensor
uint8_t pressurePin = A5; //Manifold pressure sensor pin

uint8_t tankCapacity = 60;
uint16_t lightLevel;

extern float averageRPM;
//#define TAU_GASOLINE 0.02f
//#define TAU_COOLANT 0.02f

extern float spd_m_speedometer;

#define TAU 0.01

uint32_t consumptionCounter;

// New safety/startup helpers and sample counters
#define ADC_MIN_VALID 2
#define ADC_MAX_VALID 1021
#define ADC_DENOM_EPS 1e-3f
#define STARTUP_SAMPLES 40
#define FUEL_FAULT_TOLERANCE_RATIO 0.05f

static uint16_t samples_coolant = 0;
static uint16_t samples_oil = 0;
static uint16_t samples_air = 0;
static uint16_t samples_gas = 0;
static uint16_t samples_light = 0;

// Track consecutive invalid reads
static uint16_t invalid_coolant = 0;
static uint16_t invalid_oil = 0;
static uint16_t invalid_air = 0;
static uint16_t invalid_gas = 0;

#define MAX_CONSECUTIVE_INVALID 50  // Threshold for error state

static inline bool isValidADC(float v) {
    return (v > ADC_MIN_VALID) && (v < ADC_MAX_VALID);
}

static inline float safeAnalogRead(uint8_t pin) {
    // single raw read, caller will validate
    int r = analogRead(pin);
    if (r < 0) r = 0;
    if (r > 1023) r = 1023;
    return (float)r;
}

static inline float startupAlpha(float tau, uint16_t samples) {
    // Provide faster convergence during initial samples or after reconnect
    if (samples < STARTUP_SAMPLES) {
        float a = tau * 8.0f; // amplify tau while starting
        if (a < 0.25f) a = 0.25f;
        if (a > 0.9f) a = 0.9f;
        return a;
    }
    return tau;
}

extern digifiz_pars digifiz_parameters;

void updateADCSettings()
{
    coolantB = digifiz_parameters.coolantThermistorB.value;
    oilB = digifiz_parameters.oilThermistorB.value;
    airB = digifiz_parameters.airThermistorB.value;
    R1_Coolant = digifiz_parameters.coolantThermistorDefRes.value;
    R1_Oil = digifiz_parameters.oilThermistorDefRes.value;
    R1_Ambient = digifiz_parameters.ambThermistorDefRes.value;

    tauCoolant = (float)digifiz_parameters.tauCoolant.value*TAU;
    tauOil = (float)digifiz_parameters.tauOil.value*TAU*0.1;
    tauAir = (float)digifiz_parameters.tauAir.value*TAU*0.1;
    tauGasoline = (float)digifiz_parameters.tauTank.value*TAU*0.03;
    tauGasolineConsumption = (float)digifiz_parameters.tauTank.value*TAU*0.01;
    tankCapacity = digifiz_parameters.tankCapacity.value;
}

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
    updateADCSettings();
    
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
   // Read and average several samples (same as before) but protect and use startup alpha
   uint32_t lData = 0;
   for (int i = 0; i < 7; ++i) lData += analogRead(lightSensorPin);

   // still guard against impossible huge value
   if (lData > 800) {
       // treat as invalid/read error -> reset sample counter so reconnect causes fast convergence
       samples_light = 0;
       return; // ignore this burst
   }

   uint16_t avg = (uint16_t)(lData >> 3);

   // alpha behaviour: during startup, converge faster toward measured value
   float alpha = (samples_light < STARTUP_SAMPLES) ? 0.5f : 1.0f;
   if (samples_light == 0) {
       // first valid reading: set immediately
       lightLevel = avg;
   } else {
       lightLevel = (uint16_t)((1.0f - alpha) * (float)lightLevel + alpha * (float)avg);
   }
   samples_light++;
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
    if (invalid_coolant >= MAX_CONSECUTIVE_INVALID)
        return -999.9f;
    if (coolantT<-50.0f)
        return -999.9f;
    else if (coolantT>200.0)
        return -999.9f;
    else
        return coolantT;
}

float getOilTemperature()
{
    if (invalid_oil >= MAX_CONSECUTIVE_INVALID)
        return -999.9f;
    V0 = (float)analogRead(oilPin);
    R2 = R2_Oil * V0 / (1023.0f - V0); //
    float tempT = 1.0f/(log(R2/R1_Oil)/oilB+1.0f/(25.0f+273.15f))-273.15f;
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
    float v = safeAnalogRead(coolantPin);
    if (!isValidADC(v)) {
        // invalid reading
        samples_coolant = 0;
        invalid_coolant++;
        return;
    }

    float denom = 1023.0f - v;
    if (denom < ADC_DENOM_EPS) {
        samples_coolant = 0;
        invalid_coolant++;
        return;
    }

    float R2_local = 220.0f * v / denom;
    if (!isfinite(R2_local) || R2_local <= 0.0f) {
        samples_coolant = 0;
        invalid_coolant++;
        return;
    }

    float temp1 = (log(R2_local / R1_Coolant) / coolantB);
    temp1 += 1.0f / (25.0f + 273.15f);
    if (!isfinite(temp1) || temp1 == 0.0f) {
        samples_coolant = 0;
        invalid_coolant++;
        return;
    }

    float measured = 1.0f / temp1 - 273.15f;
    if (!isfinite(measured)) {
        samples_coolant = 0;
        invalid_coolant++;
        return;
    }

    // Valid reading - reset invalid counter
    invalid_coolant = 0;

    // alpha: faster during initial samples
    float alpha = startupAlpha(tauCoolant, samples_coolant);
    // initialize if first valid sample
    if (samples_coolant == 0) coolantT = measured;
    else coolantT += alpha * (measured - coolantT);

    samples_coolant++;
}

void processOilTemperature()
{
    float v = safeAnalogRead(oilPin);
    if (!isValidADC(v)) {
        samples_oil = 0;
        invalid_oil++;
        return;
    }

    float denom = 1023.0f - v;
    if (denom < ADC_DENOM_EPS) {
        samples_oil = 0;
        invalid_oil++;
        return;
    }

    float R2_local = R2_Oil * v / denom;
    if (!isfinite(R2_local) || R2_local <= 0.0f) {
        samples_oil = 0;
        invalid_oil++;
        return;
    }

    float temp1 = (log(R2_local / R1_Oil) / oilB);
    temp1 += 1.0f / (25.0f + 273.15f);
    if (!isfinite(temp1) || temp1 == 0.0f) {
        samples_oil = 0;
        invalid_oil++;
        return;
    }

    float measured = 1.0f / temp1 - 273.15f;
    if (!isfinite(measured)) {
        samples_oil = 0;
        invalid_oil++;
        return;
    }

    // Valid reading - reset invalid counter
    invalid_oil = 0;

    float alpha = startupAlpha(tauOil, samples_oil);
    if (samples_oil == 0) oilT = measured;
    else oilT += alpha * (measured - oilT);

    samples_oil++;
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
    float v = safeAnalogRead(gasolinePin);
    if (!isValidADC(v)) {
        samples_gas = 0;
        invalid_gas++;
        return;
    }

    float denom = 1023.0f - v;
    if (denom < ADC_DENOM_EPS) {
        samples_gas = 0;
        invalid_gas++;
        return;
    }

    float tankMin = digifiz_parameters.tankMinResistance.value;
    float tankMax = digifiz_parameters.tankMaxResistance.value;
    float tankRange = tankMax - tankMin;
    if (tankRange <= 0.0f) {
        samples_gas = 0;
        invalid_gas++;
        return;
    }
    float faultSpacing = tankRange * FUEL_FAULT_TOLERANCE_RATIO;
    float lowerFaultBound = tankMin - faultSpacing;
    float upperFaultBound = tankMax + faultSpacing;

    float R2_local = 220.0f * v / denom;
    if (R2_local > upperFaultBound || R2_local < lowerFaultBound) {
        samples_gas = 0;
        invalid_gas++;
        return;
    }
    float boundedR2 = constrain(R2_local, tankMin, tankMax);

    float R2scaled = 0.0f;
    if (digifiz_parameters.option_linear_fuel.value)
    {
        R2scaled = ((float)boundedR2 - tankMin) / tankRange;
    }
    else
    {
        R2scaled = 1.0f - (1300.0f - 10.3f * boundedR2 + 0.0206f * boundedR2 * boundedR2) / 1000.0f;
    }

    if (!isfinite(R2scaled)) {
        samples_gas = 0;
        invalid_gas++;
        return;
    }

    // Valid reading - reset invalid counter
    invalid_gas = 0;

    float alphaLevel = startupAlpha(tauGasoline, samples_gas);
    float alphaFilter = startupAlpha(tauGasolineConsumption, samples_gas);

    if (samples_gas == 0) {
        // initialize both on first valid sample
        gasolineLevel = (1.0f - R2scaled);
        gasolineLevelFiltered = R2scaled;
        gasolineLevelFiltered05hour = gasolineLevelFiltered;
    } else {
        gasolineLevel += alphaLevel * ((1.0f - R2scaled) - gasolineLevel); // percents
        gasolineLevelFiltered += alphaFilter * (R2scaled - gasolineLevelFiltered); // percents
    }

    if ((millis() - consumptionCounter) > 1800000)
    {
      // half hour
      consumptionCounter = millis();
      consumptionLevel = (gasolineLevelFiltered05hour - gasolineLevelFiltered) * 2.0f;
      gasolineLevelFiltered05hour = gasolineLevelFiltered;
    }

    samples_gas++;
}

void processAmbientTemperature()
{
    float v = safeAnalogRead(airPin);
    if (!isValidADC(v)) {
        samples_air = 0;
        invalid_air++;
        return;
    }

    float denom = 1023.0f - v;
    if (denom < ADC_DENOM_EPS) {
        samples_air = 0;
        invalid_air++;
        return;
    }

    float R2_local = R2_Ambient * v / denom;
    if (!isfinite(R2_local) || R2_local <= 0.0f) {
        samples_air = 0;
        invalid_air++;
        return;
    }

    float temp1 = (log(R2_local / R1_Ambient) / airB);
    temp1 += 1.0f / (25.0f + 273.15f);
    if (!isfinite(temp1) || temp1 == 0.0f) {
        samples_air = 0;
        invalid_air++;
        return;
    }

    float measured = 1.0f / temp1 - 273.15f;
    if (!isfinite(measured)) {
        samples_air = 0;
        invalid_air++;
        return;
    }

    // Valid reading - reset invalid counter
    invalid_air = 0;

    float alpha = startupAlpha(tauAir, samples_air);
    if (samples_air == 0) airT = measured;
    else airT += alpha * (measured - airT);

    samples_air++;
}

void processFirstCoolantTemperature() //to prevent filtering from zero value
{
    float vacc = 0.0f;
    for (int i = 0; i != 100; i++) {
      vacc += safeAnalogRead(coolantPin);
    }
    vacc /= 100.0f;
    if (!isValidADC(vacc) || (1023.0f - vacc) < ADC_DENOM_EPS) {
        // leave default coolantT; set samples to zero so later good reads cause fast convergence
        samples_coolant = 0;
        return;
    }
    float R2_local = 220.0f * vacc / (1023.0f - vacc);
    float temp1 = (log(R2_local / R1_Coolant) / coolantB);
    temp1 += 1.0f / (25.0f + 273.15f);
    coolantT = (1.0f / temp1 - 273.15f);
    if (!isfinite(coolantT)) {
        samples_coolant = 0;
    } else {
        samples_coolant = STARTUP_SAMPLES; // consider this warmed-up
    }
}

void processFirstOilTemperature()
{
    float vacc = 0.0f;
    for (int i = 0; i != 100; i++) {
      vacc += safeAnalogRead(oilPin);
    }
    vacc /= 100.0f;
    if (!isValidADC(vacc) || (1023.0f - vacc) < ADC_DENOM_EPS) {
        samples_oil = 0;
        return;
    }
    float R2_local = R2_Oil * vacc / (1023.0f - vacc);
    float temp1 = (log(R2_local / R1_Oil) / oilB);
    temp1 += 1.0f / (25.0f + 273.15f);
    oilT = (1.0f / temp1 - 273.15f );
    if (!isfinite(oilT)) {
        samples_oil = 0;
    } else {
        samples_oil = STARTUP_SAMPLES;
    }
}

void processFirstGasLevel()
{
    float vacc = 0.0f;
    for (int i = 0; i != 300; i++) {
      vacc += safeAnalogRead(gasolinePin);
    }
    vacc /= 300.0f;
    if (!isValidADC(vacc) || (1023.0f - vacc) < ADC_DENOM_EPS) {
        samples_gas = 0;
        return;
    }
    float tankMin = digifiz_parameters.tankMinResistance.value;
    float tankMax = digifiz_parameters.tankMaxResistance.value;
    float tankRange = tankMax - tankMin;
    if (tankRange <= 0.0f) {
        samples_gas = 0;
        invalid_gas++;
        return;
    }
    float faultSpacing = tankRange * FUEL_FAULT_TOLERANCE_RATIO;
    float lowerFaultBound = tankMin - faultSpacing;
    float upperFaultBound = tankMax + faultSpacing;

    float R2_local = 220.0f * vacc / (1023.0f - vacc);
    if (R2_local > upperFaultBound || R2_local < lowerFaultBound) {
        samples_gas = 0;
        invalid_gas++;
        return;
    }
    R2_local = constrain(R2_local, tankMin, tankMax);
    float R2scaled = 0.0f;
    if (digifiz_parameters.option_linear_fuel.value)
    {
        R2scaled = (((float)R2_local - tankMin) / tankRange);
    }
    else
    {
        R2scaled = 1.0f - (1300.0f - 10.3f * R2_local + 0.0206f * R2_local * R2_local) / 1000.0f;
    }
    //35 = full
    //265 = empty
    gasolineLevel = 1.0f - R2scaled; //percents
    gasolineLevelFiltered = R2scaled; //percents
    gasolineLevelFiltered05hour = gasolineLevelFiltered;
    samples_gas = STARTUP_SAMPLES;
}

void processFirstAmbientTemperature()
{
    float vacc = 0.0f;
    for (int i = 0; i != 100; i++) {
      vacc += safeAnalogRead(airPin);
    }
    vacc /= 100.0f;
    if (!isValidADC(vacc) || (1023.0f - vacc) < ADC_DENOM_EPS) {
        samples_air = 0;
        return;
    }
    float R2_local = R2_Ambient * vacc / (1023.0f - vacc);
    float temp1 = (log(R2_local / R1_Ambient) / airB);
    temp1 += 1.0f / (25.0f + 273.15f);
    airT = (1.0f / temp1 - 273.15f);
    if (!isfinite(airT)) {
        samples_air = 0;
    } else {
        samples_air = STARTUP_SAMPLES;
    }
}

float getRToFuelLevel(float R)
{
  return (1300.0f-10.3f*R+0.0206f*R*R);
}

float getGasLevel()
{
    V0 = (float)analogRead(gasolinePin);
    R2 = constrain(220 * V0 / (1023.0f - V0),digifiz_parameters.tankMinResistance.value,digifiz_parameters.tankMaxResistance.value); // 330 Ohm in series with fuel sensor
    float R2scaled = 0.0f;
    if (digifiz_parameters.option_linear_fuel.value)
    {
        R2scaled = (((float)R2-digifiz_parameters.tankMinResistance.value) / (digifiz_parameters.tankMaxResistance.value - digifiz_parameters.tankMinResistance.value));
    }
    else
    {
        R2scaled = 1.0f-(1300.0f-10.3f*R2+0.0206f*R2*R2)/1000.0f;
    }
   
    gasolineLevel += tauGasoline*(R2scaled-gasolineLevel); //percents
    gasolineLevelFiltered += tauGasolineConsumption*(R2scaled-gasolineLevelFiltered); //percents
    return gasolineLevel;
}

uint8_t getLitresInTank() //0..99
{
    if (invalid_gas >= MAX_CONSECUTIVE_INVALID)
        return 0;
    return constrain(gasolineLevel*(float)tankCapacity,0,99); //where 99 of course means error
}

uint8_t getGallonsInTank() //0..99
{
    if (invalid_gas >= MAX_CONSECUTIVE_INVALID)
        return 0;
    return constrain(gasolineLevel*(float)tankCapacity*0.264172f,0,99); //where 99 of course means error 
}

uint8_t getDisplayedCoolantTemp()  //0..14, 0..16
{
  if (invalid_coolant >= MAX_CONSECUTIVE_INVALID)
      return 0;
#if defined(AUDI_DISPLAY) || defined(TRANSPORTER_DISPLAY)
    //16 LEDs
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance.value)/
            (digifiz_parameters.coolantMaxResistance.value - digifiz_parameters.coolantMinResistance.value)*16.0f),0,16);
#endif
#ifdef AUDI_RED_DISPLAY
    //17 LEDs
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance.value)/
            (digifiz_parameters.coolantMaxResistance.value - digifiz_parameters.coolantMinResistance.value)*17.0f),0,17);

#endif

#if !defined(AUDI_RED_DISPLAY) && !defined(AUDI_DISPLAY) && !defined(TRANSPORTER_DISPLAY)
    //14 LEDs
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance.value)/
            (digifiz_parameters.coolantMaxResistance.value - digifiz_parameters.coolantMinResistance.value)*14.0f),0,14); 
 
#endif            
}

uint8_t getDisplayedCoolantTempOrig()  //0..14
{
    //20 LCD segments
    return constrain((int)((coolantT-digifiz_parameters.coolantMinResistance.value)/
            (digifiz_parameters.coolantMaxResistance.value - digifiz_parameters.coolantMinResistance.value)*20.0f),0,20); 
}


float getAmbientTemperature()
{
    V0 = (float)analogRead(airPin);
    R2 = R2_Ambient * V0 / (1023.0f - V0); //
    float tempT = 1.0f/(log(R2/R1_Ambient)/airB+1.0f/(25.0f+273.15f))-273.15f;
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
