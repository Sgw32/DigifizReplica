#include "fuel_pressure.h"
#include "params.h"
#include "esp_log.h"

#define MAX_BARS 10.0f
#define TAU_FUEL_PRESSURE 0.5f

extern float constrain(float input, float max, float min);

//TODO check
uint8_t fuel_pressurePin = 10; //Manifold pressure sensor pin
float fuelPressure = 0.0f;

void initFuelPressureSensor()
{
    ESP_LOGI(LOG_TAG, "initFuelPressureSensor started");
    //pinMode(fuel_pressurePin,INPUT);
    fuelPressure = 0.0f;
    ESP_LOGI(LOG_TAG, "initFuelPressureSensor ended");
}


void processFuelPressure()
{
    //TODO
    uint16_t adcData = 0;
    for (uint8_t a=0;a!=16;a++)
    {
        adcData+=2048.0f;//analogRead(fuel_pressurePin);
    }
    adcData = constrain(adcData>>4,103,818+103)-103; // div by 16 and transpose
    fuelPressure+=TAU_FUEL_PRESSURE*((adcData/4095.0f*5.0f)*MAX_BARS/4.0f-fuelPressure); //convert to bars and filter
}

float getFuelPressure()
{
    //return fuelPressure; //0..10 bar
    return fuelPressure; //0..10 bar
}

float getFuelPressureVoltage()
{
    float intp = 2048.0f;//(float)analogRead(fuel_pressurePin);
    return intp/4095.0f*5.0f;
}
