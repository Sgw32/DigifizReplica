#include "adc.h"

float R1 = 1000;
float logR2, R2, T, oilT, airT;
float coolantB = 4000;
float V0;
float gasolineLevel;
uint8_t oil03Pin = A1; // Oil pressure 0.3
uint8_t oil08Pin = A2; // Oil pressure 0.8
uint8_t lightSensorPin = A9; // Light sensor
uint8_t coolantPin = A10; // Coolant temp sensor
uint8_t gasolinePin = A11; //Gasoline sensor
uint8_t oilPin = A14; //Gasoline sensor
uint8_t airPin = A15; //Gasoline sensor

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
    T = oilT = airT = 0.0f;
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
    Vo = (float)analogRead(coolantPin);
    R2 = R1 * Vo / (1023.0f - Vo); //
    float tempT = 1.0f/(log(R2/R1)/coolantB+1.0f/(25.0f+273.15f))-273.15f;
    if (tempT<-50.0f)
        return -999.9f;
    else if (tempT>200.0)
        return -999.9f;
    else
        return T+=0.1*(tempT-T);
}

float getOilTemperature()
{
    Vo = (float)analogRead(oilPin);
    R2 = R1 * Vo / (1023.0f - Vo); //
    float tempT = 1.0f/(log(R2/R1)/coolantB+1.0f/(25.0f+273.15f))-273.15f;
    if (tempT<-50.0f)
        return -999.9f;
    else if (tempT>200.0)
        return -999.9f;
    else
        return oilT +=0.1*(tempT-oilT);
}

float getGasLevel()
{
    Vo = (float)analogRead(gasolinePin);
    R2 = constrain(330 * Vo / (1023.0f - Vo),35,265); // 330 Ohm in series with fuel sensor
    gasoline += 0.1*(((float)R2-35.0f)/(265.0f-35.0f)-gasoline);
    return ;
}

float getAmbientTemperature()
{
    Vo = (float)analogRead(airPin);
    R2 = R1 * Vo / (1023.0f - Vo); //
    float tempT = 1.0f/(log(R2/R1)/coolantB+1.0f/(25.0f+273.15f))-273.15f;
    if (tempT<-50.0f)
        return -999.9f;
    else if (tempT>200.0)
        return -999.9f;
    else
        return airT +=0.1*(tempT-airT);
    return -999.9f;
}