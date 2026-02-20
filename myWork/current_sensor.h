#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "include/errc.h"

/**
 * @brief Performance presets to ADC timing and averaging.
 */
typedef enum {
    INA228_PERF_FAST,      // Low latency, higher noise
    INA228_PERF_BALANCED,  // Medium filtering
    INA228_PERF_PRECISION  // High filtering, slow response
} current_sensor_perf_t;

typedef struct {
    uint8_t i2c_address; // Deals with how you configure A0 and A1 pins
    float shunt_res; // in Ohms
    float max_expected_current;        //hardcode me!
    current_sensor_perf_t performance; //hardcode me!
} current_sensor_config_t;

// voltage in V, current in amps, power in watts, temp in celcius
typedef enum {
    VOLTAGE, 
    CURRENT,
    POWER,
    TEMP
} current_sensor_data_t;

/**
 * @brief Initialize the INA228 current sensor. 
 */
ti_errc_t current_sensor_init(current_sensor_config_t config);

/**
 * @brief Fetches latest specified data: voltage, current, power, or temperature.
 * @return the specified data or -1 if an error occurs. 
 */
uint32_t current_sensor_read(current_sensor_data_t data);