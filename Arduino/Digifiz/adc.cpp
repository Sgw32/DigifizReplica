#include "adc.h"

float R1 = 1000;
float logR2, R2, coolantT, oilT, airT;
float coolantB = 4000;
float oilB = 4000;
float airB = 4000;
float V0;
float gasolineLevel;

float tauCoolant, tauGasoline, tauAir, tauOil;

uint8_t oil03Pin = A1; // Oil pressure 0.3
uint8_t oil08Pin = A2; // Oil pressure 0.8
uint8_t lightSensorPin = A9; // Light sensor
uint8_t coolantPin = A10; // Coolant temp sensor
uint8_t gasolinePin = A11; //Gasoline sensor
uint8_t oilPin = A14; //Gasoline sensor
uint8_t airPin = A15; //Gasoline sensor

uint8_t tankCapacity = 60;

//#define TAU_GASOLINE 0.02f
//#define TAU_COOLANT 0.02f

#define TAU 0.01

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
    coolantT = oilT = airT = 0.0f;
    coolantB = digifiz_parameters.coolantThermistorB;
    oilB = digifiz_parameters.oilThermistorB;
    airB = digifiz_parameters.airThermistorB;
    tauCoolant = (float)digifiz_parameters.tauCoolant*TAU;
    tauOil = (float)digifiz_parameters.tauOil*TAU;
    tauAir = (float)digifiz_parameters.tauAir*TAU;
    tauGasoline = (float)digifiz_parameters.tauCoolant*TAU;
    tankCapacity = digifiz_parameters.tankCapacity;
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
    R2 = R1 * V0 / (1023.0f - V0); //
    float tempT = 1.0f/(log(R2/R1)/coolantB+1.0f/(25.0f+273.15f))-273.15f;
    if (tempT<-50.0f)
        return -999.9f;
    else if (tempT>200.0)
        return -999.9f;
    else
        return oilT +=0.1*(tempT-oilT);
}


void processCoolantTemperature()
{
    V0 = (float)analogRead(coolantPin);
    R2 = 220.0f * V0 / (1023.0f - V0); //
    float temp1 = (log(R2/R1)/coolantB);
    temp1 += 1/(25.0f+273.15f);
    coolantT += tauCoolant*(1.0f/temp1 - 273.15f - coolantT);
}

void processOilTemperature()
{
    V0 = (float)analogRead(oilPin);
    R2 = 220.0f * V0 / (1023.0f - V0); //
    float temp1 = (log(R2/R1)/oilB);
    temp1 += 1/(25.0f+273.15f);
    oilT += tauOil*(1.0f/temp1 - 273.15f - oilT);
}

void processGasLevel()
{
    V0 = (float)analogRead(gasolinePin);
    R2 = constrain(330 * V0 / (1023.0f - V0),35,265); // 330 Ohm in series with fuel sensor
    gasolineLevel += tauGasoline*(((float)R2-digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                        digifiz_parameters.tankMinResistance)-gasolineLevel); //percents
}

void processAmbientTemperature()
{
    V0 = (float)analogRead(airPin);
    R2 = 220.0f * V0 / (1023.0f - V0); 
    float temp1 = (log(R2/R1)/airB);
    temp1 += 1/(25.0f+273.15f);
    airT += tauAir*(1.0f/temp1 - 273.15f - airT);
}

float getGasLevel()
{
    V0 = (float)analogRead(gasolinePin);
    R2 = constrain(330 * V0 / (1023.0f - V0),35,265); // 330 Ohm in series with fuel sensor
    gasolineLevel += tauGasoline*(((float)R2-
              digifiz_parameters.tankMinResistance)/(digifiz_parameters.tankMaxResistance-
                                                digifiz_parameters.tankMinResistance)-gasolineLevel); //percents
    return gasolineLevel;
}

uint8_t getLitresInTank() //0..99
{
    return constrain(gasolineLevel*(float)tankCapacity,0,99); //where 99 of course means error
}

uint8_t getDisplayedCoolantTemp()  //0..14
{
    return constrain((int)((coolantT-60.0f)/(120.0f - 60.0f)*14.0f),0,14); //TODO parameters
}

float getAmbientTemperature()
{
    V0 = (float)analogRead(airPin);
    R2 = R1 * V0 / (1023.0f - V0); //
    float tempT = 1.0f/(log(R2/R1)/coolantB+1.0f/(25.0f+273.15f))-273.15f;
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
