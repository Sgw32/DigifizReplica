#ifndef TACHO_H
#define TACHO_H
#include "Arduino.h"
#include <PinChangeInterrupt.h>
#include "MedianDispertionFilter.h"

#define RPM_PIN A12 //PK4 //KL_1

/**
 * @brief Inits tacho module
 * 
 */
void initTacho();

/**
 * @brief returns filtered RPM in micros
 * 
 * @return uint32_t 
 */
uint32_t readLastRPM();

/**
 * @brief Reads the dispertion of RPM which helps to filter out unusable results
 * 
 * @return uint32_t 
 */
uint32_t getRPMDispertion();

/**
 * @brief gets RPM mean from the filter
 * 
 * @return uint32_t 
 */
uint32_t getRPMMean();
#endif
