#include "fuel_pressure.h"

#define MAX_BARS 10.0f
#define TAU_FUEL_PRESSURE 0.5f

uint8_t fuel_pressurePin = A6; //Manifold pressure sensor pin
float fuelPressure = 0.0f;

void initFuelPressureSensor()
{
    pinMode(fuel_pressurePin,INPUT);
    fuelPressure = 0.0f;
}

void processFuelPressure()
{
    uint16_t adcData = 0;
    for (uint8_t a=0;a!=16;a++)
    {
        adcData+=analogRead(fuel_pressurePin);
    }
    adcData = constrain(adcData>>4,103,818+103)-103; // div by 16 and transpose
    fuelPressure+=TAU_FUEL_PRESSURE*((adcData/1023.0f*5.0f)*MAX_BARS/4.0f-fuelPressure); //convert to bars and filter
}

float getFuelPressure()
{
    //return fuelPressure; //0..10 bar
    return fuelPressure; //0..10 bar
}

float getFuelPressureVoltage()
{
    float intp = (float)analogRead(fuel_pressurePin);
    return intp/1023.0f*5.0f;
}
