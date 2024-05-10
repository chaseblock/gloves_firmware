#include <zephyr/kernel.h>

#include "ble.h"
#include "imu.h"
#include "adc.h"

#define SAMPLES_PER_PKT 4
static uint16_t pkt_id = 0;
static size_t sample_cnt = 0;

static ble_data_t data[SAMPLES_PER_PKT];

void app_loop(struct k_work *work)
{
    ARG_UNUSED(work);

    // Update the readings
    imu_get(&data[sample_cnt].imu_data);
    read_force_sensors(&data[sample_cnt].force_data);

    data[sample_cnt].pkt_id = pkt_id++;
    sample_cnt++;

    if(sample_cnt >= SAMPLES_PER_PKT) {
        sample_cnt = 0;

        // Wait for the previous message to finish sending
        // ble_wait_for_tx_completion();
        if (ble_is_sending_data())
            return;

        ble_send_data(&data, SAMPLES_PER_PKT);
    }
}
K_WORK_DEFINE(app, app_loop);

void timer_handler(struct k_timer *unused)
{
    ARG_UNUSED(unused);
    k_work_submit(&app);
}
K_TIMER_DEFINE(app_timer, timer_handler, NULL);

int main(void)
{
    // Set up the ADC, IMU, and BLE modules, in that order
    adc_init();
    imu_init();
    ble_init();

    // Start the periodic timer
    k_timer_start(&app_timer, K_SECONDS(0), K_MSEC(100));

    return 0;
}
