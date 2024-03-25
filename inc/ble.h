/**
 * @file ble.h
 * @author Charles Block (coblock2@illinois.edu)
 * @brief The BLE driver handles all aspects of transmission/reception over the bluetooth radio
 * @version 0.1
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <stdint.h>
#include "imu.h"

typedef struct ble_data
{
    // To indicate whether we've been dropping packets or not
    uint16_t pkt_id;

    // The force sensor measurements
    uint16_t thumb_sensor;
    uint16_t index_sensor;
    uint16_t middle_sensor;
    uint16_t ring_sensor;
    uint16_t palm1_sensor;
    uint16_t palm2_sensor;

    // The IMU measurements
    imu_data_t imu_data;
} ble_data_t;

/**
 * Initialize the BLE radio.
 */
void ble_init();

/**
 * Send a packet of data over the BLE radio.
 *
 * @param data Specification of the data packet to send
 */
void ble_send_data(ble_data_t *data);

/**
 * Wait for the current message to finish sending over BLE.
 */
void ble_wait_for_tx_completion();