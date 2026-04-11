#ifndef ADC_H
#define ADC_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file adc.h
 * @brief ADC acquisition and sensor conversion helpers.
 */

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

/**
 * @brief Clamp input value into inclusive range.
 */
float constrain(float input, float min, float max);

/** @brief Initialize ADC channels and calibration. */
void initADC();
/** @brief Run periodic ADC sampling and filtering pipeline. */
void processADC();
/** @brief Refresh ADC-related settings from parameter storage. */
void updateADCSettings();
/** @brief Read raw ADC values into sensor cache. */
void read_adc_values();

/** @name Raw values (scaled 0..1024)
 *  @{
 */
uint16_t getRawCoolantTemperature();
uint16_t getRawOilTemperature();
uint16_t getRawGasLevel();
uint16_t getRawAmbientTemperature();
/** @} */

/** @name Display-oriented helper values
 *  @{
 */
uint8_t getLitresInTank(); //0..99
uint8_t getGallonsInTank(); //0..99
uint8_t getDisplayedCoolantTemp(); //0..14
uint8_t getDisplayedCoolantTempOrig(); //0..20
/** @} */


/** @name Physical units getters
 *  @{
 */
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
/** @} */

/** @brief Update filtered fuel pressure value from ADC channel. */
void processFuelPressure();
/** @brief Update barometer pressure estimate from ADC channel. */
void processBarometer();
/** @brief Update wideband lambda AFR estimate from ADC channel. */
void processWidebandLambda();

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

/** @brief Apply board-specific workaround for oil ADC channel routing. */
void reconfigOilChannel();

/** @name Raw ADC cache accessors
 *  @{
 */
int getCoolantRawADCVal(void);
int getFuelRawADCVal(void);
int getLightRawADCVal(void);
int getAmbTempRawADCVal(void);
int getOilTempRawADCVal(void);
int getIntakePressRawADCVal(void);
int getFuelPressRawADCVal(void);
/** @} */

/** @name Derived sensor resistances
 *  @{
 */
float getCoolantResistance(void);
float getOilResistance(void);
float getAmbientResistance(void);
float getFuelLevelResistance(void);
/** @} */

/** @brief Prime filters by reading first ADC sample set. */
void read_initial_adc_values();
/** @brief Log currently sampled sensor data for diagnostics. */
void log_sensor_data();

/** @brief Get bitmask indicating currently detected faulty sensors. */
DeviceSensorsFaulty getFaultyMask();

#ifdef __cplusplus
}
#endif
#endif
