#include <zephyr/bluetooth/bluetooth.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <bluetooth/services/nus.h>
#include <zephyr/logging/log.h>
#include <shell/shell_bt_nus.h>
#include <stdio.h>
#include <bluetooth/services/nus.h>

#include "ble.h"

#define NUS_WRITE_TIMEOUT K_MSEC(150)

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

K_SEM_DEFINE(nus_write_sem, 1, 1);

static bool sending_data = false;

/**
 * @brief Callback for when a bluetooth send operation is completed.
 *
 * @param nus
 * @param err
 * @param data
 * @param len
 */
static void ble_data_sent(struct bt_conn *conn)
{
    ARG_UNUSED(conn);

    sending_data = false;
    k_sem_give(&nus_write_sem);
}

/**
 * @brief Callback for when we receive bluetooth data. Currently unused.
 *
 * @param nus
 * @param data
 * @param len
 * @return uint8_t
 */
static void ble_data_received(struct bt_conn *conn,
                                 const uint8_t *data, uint16_t len)
{
    ARG_UNUSED(conn);
    ARG_UNUSED(data);
    ARG_UNUSED(len);
}

static int nus_init(void)
{
    int err;
    struct bt_nus_cb init = {
        .received = ble_data_received,
        .sent = ble_data_sent,
    };

    err = bt_nus_init(&init);
    if (err)
    {
        return err;
    }

    return err;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
}

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed,
};

/**
 * Initialize the BLE radio.
 */
void ble_init()
{
    int err;

    // Initialize the ble connection callbacks
    err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
    if (err)
    {
        return;
    }

    // Enable the bluetooth radio
    err = bt_enable(NULL);
    if (err)
    {
        return;
    }

    err = nus_init();
    if (err != 0)
    {
        return;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd,
                          ARRAY_SIZE(sd));
    if (err)
    {
        return;
    }
}

/**
 * Send a packet of data over the BLE radio.
 *
 * @param data Specification of the data packet to send
 */
void ble_send_data(ble_data_t *data, size_t sample_cnt)
{
    int err;

    // Take the semaphore to indicate that we're sending data.
    // The callback will give it back when tx is done.
    err = k_sem_take(&nus_write_sem, NUS_WRITE_TIMEOUT);

    sending_data = true;

    // Send the data
    err = bt_nus_send(NULL, (uint8_t *)data, sizeof(ble_data_t) * sample_cnt);
    if (err)
    {   
        sending_data = false;
        k_sem_give(&nus_write_sem);
    }
}

bool ble_is_sending_data()
{
    return sending_data;
}

/**
 * Wait for the current message to finish sending over BLE.
 */
void ble_wait_for_tx_completion()
{
    if (!sending_data)
    {
        // There's no send ongoing, so we can exit this before we
        // waste time with semaphores.
        return;
    }

    // Take the semaphore and then release it to synchronize
    // with the sending process.
    k_sem_take(&nus_write_sem, K_FOREVER);
    k_sem_give(&nus_write_sem);
}