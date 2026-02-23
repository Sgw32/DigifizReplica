#ifndef ADC_H
#define ADC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "params.h"


#define TAU 0.01

#define KELVIN_TO_CELSIUM 273.15f
#define BASE_TEMPERATURE 25.0f
#define ADC_UPPER_BOUND 4095.0f
#define ADC_NTC_INCORRECT_UPPER_BOUND 4092.0f
#define LIGHT_SENSOR_TAU 0.03f
#define AMB_TEMP_INTAKE_MODEL_LOWER_VAL -20.0f
#define AMB_TEMP_INTAKE_MODEL_UPPER_VAL 30.0f


typedef struct {
    int coolantRawADCVal;
    int fuelRawADCVal;
    int lightRawADCVal;
    int ambTempRawADCVal;
    int oilTempRawADCVal;
    int intakePressRawADCVal;
    int fuelPressRawADCVal;
} DigifizSensorData;


// Define struct with union inside
typedef struct DeviceSensorsFaulty{
    union {
        struct {
            uint32_t fault_status;  // 8-bit variable
        };
        struct {
            uint32_t fuel_faulty : 8;  // 1-bit variable
            uint32_t oil_faulty : 8;  // 1-bit variable
            uint32_t coolant_faulty : 8; // 1-bit variable
            uint32_t air_faulty : 8;    // 1-bit variable
        };
    };
} DeviceSensorsFaulty;

float constrain(float input, float min, float max);

void initADC();
void processADC();
void updateADCSettings();
void read_adc_values();

//RAW values 0..1024
uint16_t getRawCoolantTemperature();
uint16_t getRawOilTemperature();
uint16_t getRawGasLevel();
uint16_t getRawAmbientTemperature();

//Display helpers
uint8_t getLitresInTank(); //0..99
uint8_t getGallonsInTank(); //0..99
uint8_t getDisplayedCoolantTemp(); //0..14
uint8_t getDisplayedCoolantTempOrig(); //0..20


//Physical data values
float getCoolantTemperature(); //celsius
float getOilTemperature(); //celsius
float getRToFuelLevel(float R);
float getAmbientTemperature(); //celsius
float getFuelConsumption();

float getIntakePressure();
float getCurrentIntakeFuelConsumption();
float getIntakeVoltage();

float getFuelPressure();
float getBarometerPressure();
float getWidebandLambdaAFR();

void processFuelPressure();
void processBarometer();
void processWidebandLambda();
void processConsumptionSensor();

float getOilTemperatureFahrenheit(); //F
float getAmbientTemperatureFahrenheit(); //F

uint8_t getBrightnessLevel();
uint16_t getRawBrightnessLevel();

void processCoolantTemperature();
void processOilTemperature();
void processGasLevel();
void processAmbientTemperature();
void processBrightnessLevel();

void processFirstCoolantTemperature(); //to prevent filtering from zero value
void processFirstOilTemperature();
void processFirstGasLevel();
void processFirstAmbientTemperature();

//Errata for PCBs
void reconfigOilChannel();

// Function declarations
int getCoolantRawADCVal(void);
int getFuelRawADCVal(void);
int getLightRawADCVal(void);
int getAmbTempRawADCVal(void);
int getOilTempRawADCVal(void);
int getIntakePressRawADCVal(void);
int getFuelPressRawADCVal(void);

void read_initial_adc_values();
void log_sensor_data();

DeviceSensorsFaulty getFaultyMask();

#ifdef __cplusplus
}
#endif
#endif
