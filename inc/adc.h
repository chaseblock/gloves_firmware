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
    THUMB_SENSOR = 7,
    INDEX_SENSOR = 0,
    MIDDLE_SENSOR = 1,
    RING_SENSOR = 2,
    // LITTLE_SENSOR, // Currently unused
    PALM1_SENSOR = 5,
    PALM2_SENSOR = 6,
} sensor_position_t;

#define NUM_FORCE_SENSORS 6



// The force sensor measurements
typedef struct force_data {
    uint16_t thumb_sensor;
    uint16_t index_sensor;
    uint16_t middle_sensor;
    uint16_t ring_sensor;
    uint16_t palm1_sensor;
    uint16_t palm2_sensor;
} force_data_t;

/**
 * Initialize the ADC
 */
void adc_init();

/**
 * Read the value of a force sensor at a given position
 *
 * @param pos The position of the sensor to read from
 */
void read_force_sensors(force_data_t *data);
