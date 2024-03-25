/**
 * @file adc.h
 * @author Charles Block (coblock2@illinois.edu)
 * @brief The ADC driver handles all aspects of reading the raw values from the force sensors.
 * @version 0.1
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <stdint.h>

// Specify valid sensors to be read from. The total number of valid
// force sensors is given by NUM_FORCE_SENSORS
typedef enum sensor_position
{
    THUMB_SENSOR = 0,
    INDEX_SENSOR,
    MIDDLE_SENSOR,
    RING_SENSOR,
    // LITTLE_SENSOR, // Currently unused
    PALM1_SENSOR,
    PALM2_SENSOR,

    NUM_FORCE_SENSORS
} sensor_position_t;

/**
 * Initialize the ADC
 */
void adc_init();

/**
 * Read the value of a force sensor at a given position
 *
 * @param pos The position of the sensor to read from
 */
uint32_t adc_read_sensor(sensor_position_t pos);
