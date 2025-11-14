#include "ext_eeprom.h"
#include <string.h>
#include <stdlib.h>

ExternalEEPROM myMem;
uint8_t external_faulty;
static uint8_t memory_locked = 0;
static const size_t kStatusOffset = params_blob_size;

EEPROMLoadResult eeprom_load_result = EEPROM_NO_LOAD_ATTEMPT;

static bool blob_has_magic(const uint8_t* blob) {
    if (blob == nullptr) {
        return false;
    }
    const xparam_img_t* img = (const xparam_img_t*)blob;
    return img->header.magic == XPARAM_MAGIC;
}

static void write_internal_blob(const uint8_t* blob) {
    for (size_t i = 0; i < params_blob_size; ++i) {
        EEPROM.update(INTERNAL_OFFSET + i, blob[i]);
    }
    EEPROM.put(INTERNAL_OFFSET + kStatusOffset, digifiz_status);
}

static void write_external_blob(const uint8_t* blob) {
    if (external_faulty) {
        return;
    }
    for (size_t i = 0; i < params_blob_size; ++i) {
        myMem.write(EXTERNAL_OFFSET + i, blob[i]);
    }
    myMem.put(EXTERNAL_OFFSET + kStatusOffset, digifiz_status);
}

static bool read_internal_blob(uint8_t* blob) {
    for (size_t i = 0; i < params_blob_size; ++i) {
        blob[i] = EEPROM.read(INTERNAL_OFFSET + i);
    }
    EEPROM.get(INTERNAL_OFFSET + kStatusOffset, digifiz_status);
    return blob_has_magic(blob);
}

static bool read_external_blob(uint8_t* blob) {
    if (external_faulty) {
        return false;
    }
    for (size_t i = 0; i < params_blob_size; ++i) {
        myMem.read(EXTERNAL_OFFSET + i, &blob[i]);
    }
    myMem.get(EXTERNAL_OFFSET + kStatusOffset, digifiz_status);
    return blob_has_magic(blob);
}

static bool load_from_blob(uint8_t* blob) {
    if (!blob_has_magic(blob)) {
        return false;
    }
    if (!xparam_table_from_blob(&params_table, blob)) {
        return false;
    }
    return true;
}

void load_defaults()
{
    digifiz_parameters = digifiz_default_parameters;
    digifiz_status.mileage = DEFAULT_MILEAGE * 3600UL;
    digifiz_status.daily_mileage[0] = 0;
    digifiz_status.daily_mileage[1] = 0;
    digifiz_status.uptime = 0;
    digifiz_status.averageConsumption[0] = 0.0f;
    digifiz_status.averageConsumption[1] = 0.0f;
    digifiz_status.averageSpeed[0] = 0.0f;
    digifiz_status.averageSpeed[1] = 0.0f;
    digifiz_status.duration[0] = 0;
    digifiz_status.duration[1] = 0;

#if !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
    digifiz_parameters.rpmCoefficient.value = 3000;
#else
    digifiz_parameters.rpmCoefficient.value = 1500;
#endif
#ifdef DIESEL_MODE
    digifiz_parameters.rpmCoefficient.value = 400;
#endif
    digifiz_parameters.rpmFilterK.value = 70;
    digifiz_parameters.speedCoefficient.value = 100;
    digifiz_parameters.coolantThermistorB.value = COOLANT_THERMISTOR_B;
    digifiz_parameters.oilThermistorB.value = OIL_THERMISTOR_B;
    digifiz_parameters.airThermistorB.value = AIR_THERMISTOR_B;
    digifiz_parameters.tankMinResistance.value = 35;
    digifiz_parameters.tankMaxResistance.value = 265;
    digifiz_parameters.tauCoolant.value = 2;
    digifiz_parameters.tauOil.value = 2;
    digifiz_parameters.tauAir.value = 2;
    digifiz_parameters.tauTank.value = 2;
    digifiz_parameters.autoBrightness.value = 1;
    digifiz_parameters.brightnessLevel.value = 10;
#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
    digifiz_parameters.tankCapacity.value = 70;
#else
    digifiz_parameters.tankCapacity.value = 63;
#endif
    digifiz_parameters.mfaState.value = 0;
    digifiz_parameters.buzzerOff.value = 1;
#ifdef RPM_8000
#if defined(AUDI_DISPLAY) || defined(AUDI_RED_DISPLAY)
    digifiz_parameters.maxRPM.value = 7000;
#else
    digifiz_parameters.maxRPM.value = 8000;
#endif
#else
    digifiz_parameters.maxRPM.value = 7000;
#endif
    digifiz_parameters.mfaBlock.value = 0;
    digifiz_parameters.displayDot.value = 0;
    digifiz_parameters.backlight_on.value = 1;
    digifiz_parameters.coolantMinResistance.value = 60;
    digifiz_parameters.coolantMaxResistance.value = 120;
    digifiz_parameters.medianDispFilterThreshold.value = 65535;
    digifiz_parameters.coolantThermistorDefRes.value = COOLANT_R_AT_NORMAL_T;
    digifiz_parameters.oilThermistorDefRes.value = OIL_R_AT_NORMAL_T;
    digifiz_parameters.ambThermistorDefRes.value = AMBIENT_R_AT_NORMAL_T;
    digifiz_parameters.rpmFilterK.value = 70;
    digifiz_parameters.speedFilterK.value = 0;
    digifiz_parameters.signOptions_enable_touch_sensor.value = 1;
    digifiz_parameters.option_testmode_on.value = 0;
#ifdef FUEL_LEVEL_EXPERIMENTAL
    digifiz_parameters.option_linear_fuel.value = 0;
#else
    digifiz_parameters.option_linear_fuel.value = 1;
#endif
#ifdef MANUFACTURER_MFA_SWITCH
    digifiz_parameters.option_mfa_manufacturer.value = 1;
#endif
#ifdef GALLONS
    digifiz_parameters.option_gallons.value = 1;
#endif
#ifdef MILES
    digifiz_parameters.option_miles.value = 1;
#endif
#ifdef FAHRENHEIT
    digifiz_parameters.option_fahrenheit.value = 1;
#endif
#ifdef KELVIN
    digifiz_parameters.option_kelvin.value = 1;
#endif
}

void initEEPROM()
{
    load_defaults();
    #ifdef DISABLE_EEPROM
    return;
    #endif

    Wire.begin();
    external_faulty = !myMem.begin();

    bool loaded = false;
    uint8_t* blob = (uint8_t*)malloc(params_blob_size);
    if (blob != nullptr) {
        if (read_internal_blob(blob) && load_from_blob(blob)) {
            loaded = true;
            eeprom_load_result = EEPROM_OK1;
        } else if (read_external_blob(blob) && load_from_blob(blob)) {
            loaded = true;
            eeprom_load_result = EEPROM_OK2;
            write_internal_blob(blob);
        }
        free(blob);
    }

    if (!loaded) {
        eeprom_load_result = EEPROM_CORRUPTED;
        load_defaults();
        saveParameters();
    }
}

void saveParameters()
{
#ifdef DISABLE_EEPROM
    return;
#endif
    if (memory_locked) {
        return;
    }
    uint8_t* blob = xparam_table_to_blob(&params_table);
    if (blob == nullptr) {
        return;
    }
    cli();
    write_internal_blob(blob);
    write_external_blob(blob);
    sei();
    free(blob);
}

uint8_t getCurrentMemoryBlock()
{
    return 0;
}

void lockMemory()
{
    memory_locked = 1;
}

void unlockMemory()
{
    memory_locked = 0;
}

EEPROMLoadResult getLoadResult()
{
    return eeprom_load_result;
}
