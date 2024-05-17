#ifndef EMERGENCY_H
#define EMERGENCY_H

#include "params.h"
#include "buzzer.h"
#include "display_next.h"

#define OIL_0_3BAR_PIN A1 //PF1
#define OIL_1_8BAR_PIN A2 //PF2
#define CHECK_ENGINE_IN 65 //PJ2
#define OIL_LED_PIN 22 //PA0

/**
 * @brief Init of the module
 * 
 */
void initEmergencyModule();

/**
 * @brief Processes GPIO state of CHECK signal
 * 
 * @return uint8_t 
 */
uint8_t processCHECKEngine();

/**
 * @brief Gives emergency state from current RPM and other data(sensors)
 * 
 * @param mRPM 
 * @return uint8_t 
 */
uint8_t processOilPressure(int mRPM);

/**
 * @brief Displays the results from the data
 * 
 * @param mRPM 
 */
void checkEmergency(int mRPM);

#endif
