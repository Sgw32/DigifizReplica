#ifndef VEHICLE_DATA_H
#define VEHICLE_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cJSON.h>
#include <stdint.h>

/**
 * @brief Global buffer to hold the latest JSON string of vehicle data.
 *        It should be treated as read-only by other modules.
 *        The memory is managed within the vehicle_data module.
 */
extern char *g_jsonString;

/**
 * @brief Initializes the vehicle data module.
 *        Must be called before using any other functions.
 * 
 * @return 0 on success, -1 on failure.
 */
int initVehicleJSON();

/**
 * @brief Frees the memory allocated for `g_jsonString` and cleans up the vehicle data module.
 *        Should be called before the application exits to prevent memory leaks.
 */
void cleanup_vehicle_data();


/**
 * @brief Generates the JSON string from the current vehicle_data and updates g_jsonString.
 *        This function is thread-safe.
 * 
 * @return 0 on success, -1 on failure.
 */
int update_json_string();

/* Getter Functions */

/**
 * @brief Retrieves the current coolant temperature in Celsius.
 * 
 * @return Current coolant temperature.
 */
double get_coolant_temp_c();

/**
 * @brief Retrieves the current oil temperature in Celsius.
 * 
 * @return Current oil temperature.
 */
double get_oil_temp_c();

/**
 * @brief Retrieves the current ambient temperature in Celsius.
 * 
 * @return Current ambient temperature.
 */
double get_ambient_temp_c();

/**
 * @brief Retrieves the current speed in KMH.
 * 
 * @return Current speed.
 */
double get_speed_kmh();

/**
 * @brief Retrieves the current RPM (Revolutions Per Minute).
 * 
 * @return Current RPM.
 */
double get_rpm();

/**
 * @brief Retrieves the current fuel level in Liters.
 * 
 * @return Current fuel level.
 */
double get_fuel_level_l();

/**
 * @brief Retrieves the current uptime in hours.
 * 
 * @return Current uptime.
 */
double get_uptime_h();

/**
 * @brief Retrieves the current mileage in Kilometers.
 * 
 * @return Current mileage.
 */
double get_mileage_km();

/**
 * @brief Retrieves the current fuel consumption in Liters per 100 KM.
 * 
 * @return Current fuel consumption.
 */
double get_fuel_consumption_l100();

/**
 * @brief Retrieves the current intake voltage in Volts.
 * 
 * @return Current intake voltage.
 */
double get_intake_voltage_v();

/**
 * @brief Retrieves the current fuel pressure in bar.
 * 
 * @return Current fuel pressure.
 */
double get_fuel_pressure_bar();

/* Setter Functions */

/**
 * @brief Sets the coolant temperature in Celsius.
 *        Updates the JSON string accordingly.
 * 
 * @param temp The new coolant temperature.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_coolant_temp_c(double temp);

/**
 * @brief Sets the oil temperature in Celsius.
 *        Updates the JSON string accordingly.
 * 
 * @param temp The new oil temperature.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_oil_temp_c(double temp);

/**
 * @brief Sets the ambient temperature in Celsius.
 *        Updates the JSON string accordingly.
 * 
 * @param temp The new ambient temperature.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_ambient_temp_c(double temp);

/**
 * @brief Sets the speed in KMH.
 *        Updates the JSON string accordingly.
 * 
 * @param speed The new speed.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_speed_kmh(double speed);

/**
 * @brief Sets the RPM (Revolutions Per Minute).
 *        Updates the JSON string accordingly.
 * 
 * @param rpm The new RPM.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_rpm(double rpm);

/**
 * @brief Sets the fuel level in Liters.
 *        Updates the JSON string accordingly.
 * 
 * @param level The new fuel level.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_fuel_level_l(double level);

/**
 * @brief Sets the uptime in hours.
 *        Updates the JSON string accordingly.
 * 
 * @param uptime The new uptime.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_uptime_h(double uptime);

/**
 * @brief Sets the mileage in Kilometers.
 *        Updates the JSON string accordingly.
 * 
 * @param mileage The new mileage.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_mileage_km(double mileage);

/**
 * @brief Sets the fuel consumption in Liters per 100 KM.
 *        Updates the JSON string accordingly.
 * 
 * @param consumption The new fuel consumption.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_fuel_consumption_l100(double consumption);

/**
 * @brief Sets the intake voltage in Volts.
 *        Updates the JSON string accordingly.
 * 
 * @param voltage The new intake voltage.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_intake_voltage_v(double voltage);

/**
 * @brief Sets the fuel pressure in bar.
 *        Updates the JSON string accordingly.
 * 
 * @param pressure The new fuel pressure.
 * 
 * @return 0 on success, -1 on failure.
 */
int set_fuel_pressure_bar(double pressure);

#ifdef __cplusplus
}
#endif

#endif // VEHICLE_DATA_H
