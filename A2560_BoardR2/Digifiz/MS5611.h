/*
MS5611.h - Header file for the MS5611 Barometric Pressure & Temperature Sensor Arduino Library.

Version: 1.0.0
(c) 2014 Korneliusz Jarzebski
www.jarzebski.pl

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MS5611_h
#define MS5611_h
/**
 * @file MS5611.h
 * @brief Driver interface for the MS5611 pressure/temperature sensor.
 */

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/** @brief I2C address of the MS5611 device. */
#define MS5611_ADDRESS                (0x77)

/** @brief ADC read command. */
#define MS5611_CMD_ADC_READ           (0x00)
/** @brief Soft reset command. */
#define MS5611_CMD_RESET              (0x1E)
/** @brief Start pressure conversion command prefix (D1). */
#define MS5611_CMD_CONV_D1            (0x40)
/** @brief Start temperature conversion command prefix (D2). */
#define MS5611_CMD_CONV_D2            (0x50)
/** @brief PROM coefficient read command prefix. */
#define MS5611_CMD_READ_PROM          (0xA2)

/**
 * @brief Oversampling options supported by the MS5611.
 */
typedef enum
{
    /** @brief Highest precision and longest conversion time. */
    MS5611_ULTRA_HIGH_RES   = 0x08,
    /** @brief High resolution mode. */
    MS5611_HIGH_RES         = 0x06,
    /** @brief Standard resolution mode. */
    MS5611_STANDARD         = 0x04,
    /** @brief Low-power mode with faster conversions. */
    MS5611_LOW_POWER        = 0x02,
    /** @brief Lowest power, lowest resolution mode. */
    MS5611_ULTRA_LOW_POWER  = 0x00
} ms5611_osr_t;

/**
 * @brief MS5611 sensor driver.
 */
class MS5611
{
    public:
    /**
     * @brief Initializes I2C and reads sensor calibration coefficients.
     * @param osr Selected oversampling mode.
     * @return true if sensor initialization succeeded.
     */
	bool begin(ms5611_osr_t osr = MS5611_HIGH_RES);
    /** @brief Reads raw uncompensated temperature ADC counts. */
	uint32_t readRawTemperature(void);
    /** @brief Reads raw uncompensated pressure ADC counts. */
	uint32_t readRawPressure(void);
    /** @brief Reads temperature in degrees Celsius. */
	double readTemperature(bool compensation = false);
    /** @brief Reads pressure in Pa. */
	int32_t readPressure(bool compensation = false);
    /** @brief Calculates altitude from pressure and sea-level reference. */
	double getAltitude(double pressure, double seaLevelPressure = 101325);
    /** @brief Calculates sea-level pressure from local pressure and altitude. */
	double getSeaLevel(double pressure, double altitude);
    /** @brief Updates active oversampling mode. */
	void setOversampling(ms5611_osr_t osr);
    /** @brief Returns current oversampling mode. */
	ms5611_osr_t getOversampling(void);

    private:

	uint16_t fc[6];
	uint8_t ct;
	uint8_t uosr;
	int32_t TEMP2;
	int64_t OFF2, SENS2;

	void reset(void);
	void readPROM(void);

	uint16_t readRegister16(uint8_t reg);
	uint32_t readRegister24(uint8_t reg);
};

#endif
