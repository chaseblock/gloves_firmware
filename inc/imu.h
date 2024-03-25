/**
 * @file imu.h
 * @author Charles Block (coblock2@illinois.edu)
 * @brief The IMU driver handles all aspects of configuring and reading data from the on-board IMU
 * @version 0.1
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <stdio.h>

typedef struct imu_data
{
    // Temperature data
    uint16_t temp;

    // Gyro data
    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t gyro_z;

    // Accelerometer data
    uint16_t accel_x;
    uint16_t accel_y;
    uint16_t accel_z;
} imu_data_t;

/**
 * Initialize the IMU
 */
void imu_init();

/**
 * Get the current data values from the IMU
 *
 * @param rdata A pointer to a data structure to store the results in
 */
void imu_get(imu_data_t *rdata);