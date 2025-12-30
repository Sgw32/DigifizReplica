#include "adc.h"
#include <math.h>
#include "params.h"
#include "setup.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "millis.h"

#ifndef NEW_REVISION
float R2_Ambient = 3300.0f; //for Coolant
#else
float R2_Ambient = 1000.0f; //for Coolant
#endif

float R2_Coolant = 220;

#ifdef OIL_RES_10000
float R2_Oil = 3300;
#else
float R2_Oil = 220;
#endif

float R1_Coolant = COOLANT_R_AT_NORMAL_T; //for Coolant
float R1_Oil = OIL_R_AT_NORMAL_T; //for Coolant
float R1_Ambient = AMBIENT_R_AT_NORMAL_T; //for Coolant
const uint8_t lightSensorChannel = ADC_CHANNEL_4; // Light sensor
const uint8_t coolantChannel = ADC_CHANNEL_0; // Coolant temp sensor
const uint8_t gasolineChannel = ADC_CHANNEL_1; //Gasoline sensor
const uint8_t oilChannel = ADC_CHANNEL_7; //Oil temp sensor
const uint8_t airChannel = ADC_CHANNEL_3; //Air temp sensor
const uint8_t intakePressureChannel = ADC_CHANNEL_5; //Manifold pressure sensor pin
const uint8_t fuelPressureChannel = ADC_CHANNEL_6; //Manifold pressure sensor pin

float logR2 = 0.0f;
float R2 = 0.0f;
float Rseries_Oil = 0.0f;
float coolantT = 0.0f;
float oilT = 0.0f;
float airT = 0.0f;
float coolantB = 4000;
float oilB = 4000;
float airB = 4000;
float V0;
float gasolineLevel,gasolineLevelFiltered,gasolineLevelFiltered05hour;
float tauCoolant, tauGasoline, tauAir, tauOil,tauGasolineConsumption;
float consumptionLevel;

static DeviceSensorsFaulty faulty_status = {.fault_status = 0};

uint8_t tankCapacity = 55;
float lightLevel = 0;
uint32_t consumptionCounter;

extern float averageRPM;
extern float spd_m_speedometer;

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
};
adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
};

#define OIL_CHANNEL_ATTEN ADC_ATTEN_DB_2_5 

adc_oneshot_chan_cfg_t config_oil_temp_channel = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = OIL_CHANNEL_ATTEN,
};
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
adc_cali_handle_t adc1_cali_chan1_handle = NULL;
adc_cali_handle_t adc1_cali_chan2_handle = NULL;
adc_cali_handle_t adc1_cali_chan3_handle = NULL;
adc_cali_handle_t adc1_cali_chan4_handle = NULL;
adc_cali_handle_t adc1_cali_chan5_handle = NULL;
adc_cali_handle_t adc1_cali_chan6_handle = NULL;
static DigifizSensorData adc_raw;

// Read ADC values from multiple pins
void read_adc_values() {
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, lightSensorChannel, &adc_raw.lightRawADCVal));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, coolantChannel, &adc_raw.coolantRawADCVal));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, gasolineChannel, &adc_raw.fuelRawADCVal));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, oilChannel, &adc_raw.oilTempRawADCVal));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, airChannel, &adc_raw.ambTempRawADCVal));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, intakePressureChannel, &adc_raw.intakePressRawADCVal));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, fuelPressureChannel, &adc_raw.fuelPressRawADCVal));
}

void log_sensor_data() {
    // Define a tag for the log
    const char *TAG = "SENSOR_DATA";
    
    ESP_LOGI(TAG, "coolantRawADCVal: %d", adc_raw.coolantRawADCVal);
    ESP_LOGI(TAG, "fuelRawADCVal: %d", adc_raw.fuelRawADCVal);
    ESP_LOGI(TAG, "lightRawADCVal: %d", adc_raw.lightRawADCVal);
    ESP_LOGI(TAG, "ambTempRawADCVal: %d", adc_raw.ambTempRawADCVal);
    ESP_LOGI(TAG, "oilTempRawADCVal: %d", adc_raw.oilTempRawADCVal);
    ESP_LOGI(TAG, "intakePressRawADCVal: %d", adc_raw.intakePressRawADCVal);
    ESP_LOGI(TAG, "fuelPressRawADCVal: %d", adc_raw.fuelPressRawADCVal);
}

void read_initial_adc_values() {
    int adc = 0;
    for (int i=0;i!=256;i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, lightSensorChannel, &adc));
        adc_raw.lightRawADCVal+=adc;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, coolantChannel, &adc));
        adc_raw.coolantRawADCVal+=adc;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, gasolineChannel, &adc));
        adc_raw.fuelRawADCVal+=adc;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, oilChannel, &adc));
        adc_raw.oilTempRawADCVal+=adc;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, airChannel, &adc));
        adc_raw.ambTempRawADCVal+=adc;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, intakePressureChannel, &adc));
        adc_raw.intakePressRawADCVal+=adc;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, fuelPressureChannel, &adc));
        adc_raw.fuelPressRawADCVal+=adc;
    }
    adc_raw.lightRawADCVal/=256;
    adc_raw.coolantRawADCVal/=256;
    adc_raw.fuelRawADCVal/=256;
    adc_raw.oilTempRawADCVal/=256;
    adc_raw.ambTempRawADCVal/=256;
    adc_raw.intakePressRawADCVal/=256;
    adc_raw.fuelPressRawADCVal/=256;
    if (adc_raw.coolantRawADCVal>0)
        processFirstCoolantTemperature();
    if (adc_raw.oilTempRawADCVal>0)
        processFirstOilTemperature();
    if (adc_raw.fuelRawADCVal>0)
        processFirstGasLevel();
    if (adc_raw.ambTempRawADCVal>0)
        processFirstAmbientTemperature();
}

static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(LOG_TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(LOG_TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(LOG_TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

float constrain(float input, float min, float max)
{
    if (input>max)
        return max;
    if (input<min)
        return min;
    return input;
}

void reconfigOilChannel()
{
    if (digifiz_parameters.tempOptions_oil_atten.value)
    {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, oilChannel, &config_oil_temp_channel));
    }
    else
    {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, oilChannel, &config));
    }
}

DeviceSensorsFaulty getFaultyMask()
{
    return faulty_status;
}

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

    if (digifiz_parameters.oilThermistorPullUpRes.value>0)
        R2_Oil = (float)digifiz_parameters.oilThermistorPullUpRes.value; //3300 or 3300 || 220
    Rseries_Oil = (float)digifiz_parameters.oilThermistorSeriesRes.value;
    if (digifiz_parameters.ambThermistorPullUpRes.value>0)
        R2_Ambient = (float)digifiz_parameters.ambThermistorPullUpRes.value; //3300
    if (digifiz_parameters.coolantThermistorPullUpRes.value>0)
        R2_Coolant = (float)digifiz_parameters.coolantThermistorPullUpRes.value; //220   
}
// Initialize the ADC
void initADC() {
    ESP_LOGI(LOG_TAG, "initADC started");
    // Implementation placeholder
    coolantT = oilT = airT = 0.0;
    lightLevel = 0;
    updateADCSettings();

    //-------------ADC1 Init---------------//
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    //-------------ADC1 Config---------------//
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, lightSensorChannel, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, coolantChannel, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, gasolineChannel, &config));
    reconfigOilChannel();
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, airChannel, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, intakePressureChannel, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, fuelPressureChannel, &config));
    //-------------ADC1 Calibration Init---------------//

    adc_calibration_init(ADC_UNIT_1, lightSensorChannel, ADC_ATTEN_DB_12,    &adc1_cali_chan0_handle);
    adc_calibration_init(ADC_UNIT_1, coolantChannel, ADC_ATTEN_DB_12,        &adc1_cali_chan1_handle);
    adc_calibration_init(ADC_UNIT_1, gasolineChannel, ADC_ATTEN_DB_12,       &adc1_cali_chan2_handle);
    if (digifiz_parameters.tempOptions_oil_atten.value)
    {
        adc_calibration_init(ADC_UNIT_1, oilChannel, OIL_CHANNEL_ATTEN,            &adc1_cali_chan3_handle);
    }
    else
    {
        adc_calibration_init(ADC_UNIT_1, oilChannel, ADC_ATTEN_DB_12,            &adc1_cali_chan3_handle);
    }
    adc_calibration_init(ADC_UNIT_1, airChannel, ADC_ATTEN_DB_12,            &adc1_cali_chan4_handle);
    adc_calibration_init(ADC_UNIT_1, intakePressureChannel, ADC_ATTEN_DB_12, &adc1_cali_chan5_handle);
    adc_calibration_init(ADC_UNIT_1, fuelPressureChannel, ADC_ATTEN_DB_12,   &adc1_cali_chan6_handle);

    for (uint8_t i=0;i!=3;i++)
    {
        read_adc_values();
    }
    //read_initial_adc_values();
    //log_sensor_data();
    //Init values:
    
    ESP_LOGI(LOG_TAG, "initADC ended");
}

void processADC()
{
    //ESP_LOGI(LOG_TAG, "process ADC");
    read_adc_values();
    processGasLevel();
    processCoolantTemperature();
    processOilTemperature();
    processAmbientTemperature();
    processBrightnessLevel();
    #ifdef FUEL_PRESSURE_SENSOR
    processFuelPressure();
    #endif
    //ESP_LOGI(LOG_TAG, "process ADC ended");
}

// Get the raw coolant temperature
uint16_t getRawCoolantTemperature() {
    // Implementation placeholder
    return adc_raw.coolantRawADCVal;
}

// Get the raw oil temperature
uint16_t getRawOilTemperature() {
    // Implementation placeholder
    return adc_raw.oilTempRawADCVal;
}

// Get the raw gas level
uint16_t getRawGasLevel() {
    // Implementation placeholder
    return adc_raw.fuelRawADCVal;
}

// Get the raw ambient temperature
uint16_t getRawAmbientTemperature() {
    // Implementation placeholder
    return adc_raw.ambTempRawADCVal;
}

// Get the liters of fuel in the tank
uint8_t getLitresInTank() {
    return constrain(gasolineLevel*(float)tankCapacity,0,99); //where 99 of course means error
}

// Get the gallons of fuel in the tank
uint8_t getGallonsInTank() {
    return constrain(gasolineLevel*(float)tankCapacity*0.264172f,0,99); //where 99 of course means error 
}

// Get the displayed coolant temperature
uint8_t getDisplayedCoolantTemp() {
    static uint8_t alarm = 0;
    static uint32_t lastMillis = 0;
    uint8_t minSegments = digifiz_parameters.tempOptions_sensor_connected_ind.value ? 1 : 0;
    //if minSegments was set to 1 (connected int) - and sensor not connected, reset it anyway. 
    if (getFaultyMask().coolant_faulty==255)
    {
        if ((millis()-lastMillis)>TEMPERATURE_FAULTY_PERIOD)
        {
            if (alarm)
                alarm = 0;
            else
                alarm = 1;
            lastMillis = millis();
        }
        //If sensor not connected - return 14 segments
        if (alarm)
        {
            return 0;
        }
        else
        {
            return 14;
        }
    }
    else
    {
        if ((coolantT>120.0f)&&digifiz_parameters.tempOptions_alarm_function.value)
        {
            if ((millis()-lastMillis)>TEMPERATURE_ALARM_PERIOD)
            {
                if (alarm)
                    alarm = 0;
                else
                    alarm = 1;
            }
            lastMillis = millis();
        }
        else
        {
            //If alarm conditions not active, reset alarm
            alarm = 0;
        }
        //If sensor not connected
        if (coolantT<-60.0f)
            minSegments = 0;
        //If alarm active - return 0 segments(it is switched in TEMPERATURE_ALARM_PERIOD freq)
        if (alarm)
            return 0;
        
        #ifdef AUDI_DISPLAY
        //16 LEDs
        return (int)constrain((float)((coolantT-digifiz_parameters.coolantMin.value)/
                (digifiz_parameters.coolantMax.value - digifiz_parameters.coolantMin.value)*16.0f),0,16.0f);
        #endif
        #ifdef AUDI_RED_DISPLAY
            //17 LEDs
            return (int)constrain((float)((coolantT-digifiz_parameters.coolantMin.value)/
                    (digifiz_parameters.coolantMax.value - digifiz_parameters.coolantMin.value)*17.0f),0,17.0f);

        #endif

        #if !defined(AUDI_RED_DISPLAY) && !defined(AUDI_DISPLAY)
            //14 LEDs
            return (int)constrain((float)((coolantT-digifiz_parameters.coolantMin.value)/
                    (digifiz_parameters.coolantMax.value - digifiz_parameters.coolantMin.value)*14.0f),minSegments,14.0f); 
        
        #endif   
    }
}

// Get the original displayed coolant temperature
uint8_t getDisplayedCoolantTempOrig() {
    //20 LCD segments
    return (int)constrain((float)((coolantT-digifiz_parameters.coolantMin.value)/
            (digifiz_parameters.coolantMax.value - digifiz_parameters.coolantMin.value)*20.0f),0,20.0f); 
}

// Get the coolant temperature in Celsius
float getCoolantTemperature() {
    if (coolantT<-50.0f)
        return -999.9f;
    else if (coolantT>200.0)
        return -999.9f;
    else
        return coolantT;
}

// Get the oil temperature in Celsius
float getOilTemperature() {
    if (oilT<-50.0f)
    {
        oilT = -60.0f;
        return -999.9f;
    }
    else if (oilT>200.0)
    {
        oilT = 210.0f;
        return -999.9f;
    }
    return oilT;
}

// Convert resistance to fuel level
float getRToFuelLevel(float R) {
    return (1300.0f-10.3f*R+0.0206f*R*R);
}

// Get the ambient temperature in Celsius
float getAmbientTemperature() 
{
    if (airT<-50.0f)
    {
        airT = -60.0f;
        return -999.9f;
    }
    else if (airT>200.0)
    {
        airT = 210.0f;
        return -999.9f;
    }
    return airT;
}

// Get the fuel consumption
float getFuelConsumption() 
{
    return constrain(consumptionLevel,0,1.0f);
}

// Get the intake pressure
float getIntakePressure() {
    static float intp=0;
    V0 = adc_raw.intakePressRawADCVal;
    if (digifiz_parameters.signalOptions_enable_consumption_sensor.value)
    {
        intp+=(V0-intp)*0.1f;
    }

#ifdef FUEL_CONSUMPTION_TESTMODE
   intp = 512;
#endif
   
    return 84749.0f-20152.0f*intp/ADC_UPPER_BOUND*5.0f;
}

// Get the current intake fuel consumption
float getCurrentIntakeFuelConsumption() {
    //http://sergeyk.kiev.ua/avto/car_fuel_calc/
    //https://github.com/oesmith/obdgpslogger/blob/master/doc/mpg-calculation
    float kP = constrain(getIntakePressure()/1000.0f,0.0f,120.0f); // pressure in kPa 
    float lp100km = 0.0f;
    if (kP>0)
    {
      float intakeT = constrain(getAmbientTemperature(),AMB_TEMP_INTAKE_MODEL_LOWER_VAL,
                                                        AMB_TEMP_INTAKE_MODEL_UPPER_VAL)+KELVIN_TO_CELSIUM; //intakeT in K
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
}

// Get the intake voltage
float getIntakeVoltage() {
    float intp = 2048.0f;//(float)analogRead(pressurePin);
    return intp/ADC_UPPER_BOUND*5.0f;
}

// Get the oil temperature in Fahrenheit
float getOilTemperatureFahrenheit() {
    float res = getOilTemperature();
    return (res*1.8f)+32.0f;
}

// Get the ambient temperature in Fahrenheit
float getAmbientTemperatureFahrenheit() {
    float res = getAmbientTemperature();
    return (res*1.8f)+32.0f;
}

// Get the brightness level
uint8_t getBrightnessLevel() {
    lightLevel += (getRawBrightnessLevel()-lightLevel)*LIGHT_SENSOR_TAU;
    float m_lightLevel = lightLevel;
    if (digifiz_parameters.signalOptions_invert_light_input.value)
    {
        m_lightLevel = constrain((float)digifiz_parameters.brightnessSignalMax.value - m_lightLevel,
                                 0,
                                 (float)digifiz_parameters.brightnessSignalMax.value);
    }

    float minSig = (float)digifiz_parameters.brightnessSignalMin.value;
    float maxSig = (float)digifiz_parameters.brightnessSignalMax.value;
    float minBr = (float)digifiz_parameters.brightnessMin.value;
    float maxBr = (float)digifiz_parameters.brightnessMax.value;

    float level;
    if (m_lightLevel <= minSig)
        level = minBr;
    else if (m_lightLevel >= maxSig)
        level = maxBr;
    else
        level = minBr + (m_lightLevel - minSig) * (maxBr - minBr) / (maxSig - minSig);

    if (level > digifiz_parameters.brightnessLevel.value)
        level = digifiz_parameters.brightnessLevel.value;

    return (uint8_t)constrain(level, 0, 255);
}

// Get the raw brightness level
uint16_t getRawBrightnessLevel() {
    return adc_raw.lightRawADCVal;
}

// Process coolant temperature data
void processCoolantTemperature() {
    V0 = adc_raw.coolantRawADCVal;
    if (V0<ADC_NTC_INCORRECT_UPPER_BOUND)
    {
        R2 = R2_Coolant * V0 / (ADC_UPPER_BOUND - V0); //
        if (R2>0)
        {
            faulty_status.coolant_faulty = 0;
            float temp1 = (log(R2/R1_Coolant)/coolantB);
            temp1 += 1/(BASE_TEMPERATURE+KELVIN_TO_CELSIUM);
            coolantT += tauCoolant*(1.0f/temp1 - KELVIN_TO_CELSIUM - coolantT);
        }
        else
        {
            if (faulty_status.coolant_faulty < 255)
                faulty_status.coolant_faulty += 1;
        }
    }
    else
    {
        if (faulty_status.coolant_faulty < 255)
            faulty_status.coolant_faulty += 1;
    }
}

// Process oil temperature data
void processOilTemperature() {
    V0 = adc_raw.oilTempRawADCVal;
    if (V0<ADC_NTC_INCORRECT_UPPER_BOUND)
    {
        float R_total = R2_Oil * V0 / (ADC_UPPER_BOUND - V0); //
        R2 = R_total - Rseries_Oil;
        if (R2>0)
        {
            float temp1 = (log(R2/R1_Oil)/oilB);
            temp1 += 1/(BASE_TEMPERATURE+KELVIN_TO_CELSIUM);
            oilT += tauOil*(1.0f/temp1 - KELVIN_TO_CELSIUM - oilT);
            faulty_status.oil_faulty = 0;
        }
        else
        {
            if (faulty_status.oil_faulty < 255)
                faulty_status.oil_faulty += 1;
        }
    }
    else
    {
        if (faulty_status.oil_faulty < 255)
            faulty_status.oil_faulty += 1;
    }
}

// Process gas level data
void processGasLevel() {
    //TODO add values
    V0 = adc_raw.fuelRawADCVal;
    //R2 = constrain(220 * V0 / (ADC_UPPER_BOUND - V0),digifiz_parameters.tankMinResistance.value,digifiz_parameters.tankMaxResistance.value); // 220 Ohm in series with fuel sensor
    R2 = 220.0f * V0 / (ADC_UPPER_BOUND - V0);

    float tankMin = digifiz_parameters.tankMinResistance.value;
    float tankMax = digifiz_parameters.tankMaxResistance.value;
    float tankRange = tankMax - tankMin;
    const float faultRatio = 0.05f; // allow 5% tolerance before marking as faulty
    if (tankRange <= 0.0f)
    {
        if (faulty_status.fuel_faulty < 255)
            faulty_status.fuel_faulty += 1;
        return;
    }
    float faultSpacing = tankRange * faultRatio;
    float lowerFaultBound = tankMin - faultSpacing;
    float upperFaultBound = tankMax + faultSpacing;

    //TODO do not set faulty immediately
    if (R2 > upperFaultBound)
    {
        if (faulty_status.fuel_faulty < 255)
            faulty_status.fuel_faulty += 1;
        return;
    }

    //TODO do not set faulty immediately
    if (R2 < lowerFaultBound)
    {
        if (faulty_status.fuel_faulty < 255)
            faulty_status.fuel_faulty += 1;
        return;
    }
    
    faulty_status.fuel_faulty = 0;

    float R2scaled = 0.0f;
    float boundedR2 = R2;
    boundedR2 = fminf(fmaxf(boundedR2, tankMin), tankMax);

    if (digifiz_parameters.option_linear_fuel.value)
    {
        R2scaled = ((boundedR2 - tankMin) / tankRange);
    }
    else
    {
        //This formula is valid only for range tankMinResistance/tankMaxResistance 35/265
        //So if range is different, adjust R2 accordingly
        R2 = boundedR2 - digifiz_parameters.tankMinResistance.value; //0-230 Range
        //Adjust for new range:
        //equals 1 if they are 265/35, otherwise adjusted for range 265/35:
        R2 *= (265.0f-35.0f)/(digifiz_parameters.tankMaxResistance.value-digifiz_parameters.tankMinResistance.value);
        R2 += 35.0f; //New range: 265-35
        R2scaled = 1.0f - (1300.0f-10.3f*R2+0.0206f*R2*R2)/1000.0f; //This is a polynome calculated using VAG sensor gauge
    }
    //printf("ADC fuel: %f %f\n",V0, gasolineLevel);
    gasolineLevel += tauGasoline*((1.0f-R2scaled)-gasolineLevel); //percents
    gasolineLevelFiltered += tauGasolineConsumption*(R2scaled-gasolineLevelFiltered); //percents
}

// Process ambient temperature data
void processAmbientTemperature() {
    V0 = adc_raw.ambTempRawADCVal;
    if (V0<ADC_NTC_INCORRECT_UPPER_BOUND)
    {
        R2 = R2_Ambient * V0 / (ADC_UPPER_BOUND - V0); 
        if (R2>0)
        {
            float temp1 = (log(R2/R1_Ambient)/airB);
            temp1 += 1/(BASE_TEMPERATURE+KELVIN_TO_CELSIUM);
            if (temp1>0)
            {
                airT += tauAir*(1.0f/temp1 - KELVIN_TO_CELSIUM - airT);
            }
            faulty_status.air_faulty = 0;
        }
        else
        {
            if (faulty_status.air_faulty < 255)
                faulty_status.air_faulty += 1;
        }
    }
    else
    {
        if (faulty_status.air_faulty < 255)
            faulty_status.air_faulty += 1;
    }
    //printf("ADC AMBT: %f %f\n",V0, temp1);
}

// Process brightness level data
void processBrightnessLevel() {
    // Implementation placeholder
}

// Process the first coolant temperature data
void processFirstCoolantTemperature() {
    V0 = adc_raw.coolantRawADCVal;
    if (V0<ADC_NTC_INCORRECT_UPPER_BOUND)
    {
        R2 = 220.0f * V0 / (ADC_UPPER_BOUND - V0); //
        if (R2>0)
        {
            float temp1 = (log(R2/R1_Coolant)/coolantB);
            temp1 += 1/(BASE_TEMPERATURE+KELVIN_TO_CELSIUM);
            coolantT = (1.0f/temp1 - KELVIN_TO_CELSIUM);
            faulty_status.coolant_faulty = 0;
        }
        else
        {
            if (faulty_status.coolant_faulty < 255)
                faulty_status.coolant_faulty += 1;
        }
    }
    else
    {
        if (faulty_status.coolant_faulty < 255)
            faulty_status.coolant_faulty += 1;
    }
}

// Process the first oil temperature data
void processFirstOilTemperature() {
    V0 = adc_raw.oilTempRawADCVal;
    if (V0<ADC_NTC_INCORRECT_UPPER_BOUND)
    {
        float R_total = R2_Oil * V0 / (ADC_UPPER_BOUND - V0); //
        R2 = R_total - Rseries_Oil;
        if (R2>0)
        {
            float temp1 = (log(R2/R1_Oil)/oilB);
            temp1 += 1/(BASE_TEMPERATURE+KELVIN_TO_CELSIUM);
            oilT = (1.0f/temp1 - KELVIN_TO_CELSIUM );
            faulty_status.oil_faulty = 0;
        }
        else
        {
            if (faulty_status.oil_faulty < 255)
                faulty_status.oil_faulty += 1;
        }
    }
    else
    {
        if (faulty_status.oil_faulty < 255)
            faulty_status.oil_faulty += 1;
    }
}

// Process the first gas level data
void processFirstGasLevel() {
    V0 = adc_raw.fuelRawADCVal;
    float R2scaled = 0.0f;
    R2 = constrain(220 * V0 / (ADC_UPPER_BOUND - V0),digifiz_parameters.tankMinResistance.value,digifiz_parameters.tankMaxResistance.value); // 330 Ohm in series with fuel sensor
    if (digifiz_parameters.option_linear_fuel.value)
    {
        R2scaled = (((float)R2-
              digifiz_parameters.tankMinResistance.value)/(digifiz_parameters.tankMaxResistance.value-
                                                digifiz_parameters.tankMinResistance.value));
    }
    else
    {
        R2scaled = 1.0f - (1300.0f-10.3f*R2+0.0206f*R2*R2)/1000.0f;
    }
    
    //35 = full
    //265 = empty
    gasolineLevel = 1.0f - R2scaled; //percents
    gasolineLevelFiltered = R2scaled; //percents
}

// Process the first ambient temperature data
void processFirstAmbientTemperature() {
    printf("First AMBT:%f", airT);
    V0 = adc_raw.ambTempRawADCVal;
    if (V0<ADC_NTC_INCORRECT_UPPER_BOUND)
    {
        R2 = R2_Ambient * V0 / (ADC_UPPER_BOUND - V0); 
        if (R2>0)
        {
            float temp1 = (log(R2/R1_Ambient)/airB);
            temp1 += 1/(BASE_TEMPERATURE+KELVIN_TO_CELSIUM);
            airT = temp1;
            printf("First after AMBT:%f", airT);
            faulty_status.air_faulty = 0;
        }
        else
        {
            if (faulty_status.air_faulty < 255)
                faulty_status.air_faulty += 1;
        }
    }
    else
    {
        if (faulty_status.air_faulty < 255)
            faulty_status.air_faulty += 1;
    }
}

// Function definitions
int getCoolantRawADCVal(void) {
    return adc_raw.coolantRawADCVal;
}

int getFuelRawADCVal(void) {
    return adc_raw.fuelRawADCVal;
}

int getLightRawADCVal(void) {
    return adc_raw.lightRawADCVal;
}

int getAmbTempRawADCVal(void) {
    return adc_raw.ambTempRawADCVal;
}

int getOilTempRawADCVal(void) {
    return adc_raw.oilTempRawADCVal;
}

int getIntakePressRawADCVal(void) {
    return adc_raw.intakePressRawADCVal;
}

int getFuelPressRawADCVal(void) {
    return adc_raw.fuelPressRawADCVal;
}
