#include <zephyr/kernel.h>

#include "ble.h"
#include "imu.h"
#include "adc.h"

int main(void)
{
    // Some overhead stuff
    // if (IS_ENABLED(CONFIG_SETTINGS)) {
    // 	settings_load();
    // }

    // Set up the ADC, IMU, and BLE modules, in that order
    adc_init();
    imu_init();
    ble_init();

    ble_data_t data = {.pkt_id = 0xDEAD};

    // Send a test packet.
    // TODO: replace with actual data
    while (1)
    {
        imu_get(&data.imu_data);
        ble_send_data(&data);
        ble_wait_for_tx_completion();
        data.pkt_id += 1;
    }

    return 0;
}
