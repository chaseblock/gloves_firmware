#include "imu.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

// Define the register mapping of the device
#define REG_FUNC_CFG_ACCESS 0x01
#define REG_PIN_CTRL 0x02
#define REG_S4S_TPH_L 0x04
#define REG_S4S_TPH_H 0x05
#define REG_S4S_RR 0x06
#define REG_FIFO_CTRL1 0x07
#define REG_FIFO_CTRL2 0x08
#define REG_FIFO_CTRL3 0x09
#define REG_FIFO_CTRL4 0x0A
#define REG_COUNTER_BDR_REG1 0x0B
#define REG_COUNTER_BDR_REG2 0x0C
#define REG_INT1_CTRL 0x0D
#define REG_INT2_CTRL 0x0E
#define REG_WHO_AM_I 0x0F
#define REG_CTRL1_XL 0x10
#define REG_CTRL2_G 0x11
#define REG_CTRL3_C 0x12
#define REG_CTRL4_C 0x13
#define REG_CTRL5_C 0x14
#define REG_CTRL6_C 0x15
#define REG_CTRL7_G 0x16
#define REG_CTRL8_XL 0x17
#define REG_CTRL9_XL 0x18
#define REG_CTRL10_C 0x19
#define REG_ALL_INT_SRC 0x1A
#define REG_WAKE_UP_SRC 0x1B
#define REG_TAP_SRC 0x1C
#define REG_D6D_SRC 0x1D
#define REG_STATUS_REG 0x1E
#define REG_OUT_TEMP_L 0x20
#define REG_OUT_TEMP_H 0x21
#define REG_OUTX_L_G 0x22
#define REG_OUTX_H_G 0x23
#define REG_OUTY_L_G 0x24
#define REG_OUTY_H_G 0x25
#define REG_OUTZ_L_G 0x26
#define REG_OUTZ_H_G 0x27
#define REG_OUTX_L_A 0x28
#define REG_OUTX_H_A 0x29
#define REG_OUTY_L_A 0x2A
#define REG_OUTY_H_A 0x2B
#define REG_OUTZ_L_A 0x2C
#define REG_OUTZ_H_A 0x2D
#define REG_EMB_FUNC_STATUS_MAINPAGE 0x35
#define REG_FSM_STATUS_A_MAINPAGE 0x36
#define REG_FSM_STATUS_B_MAINPAGE 0x37
#define REG_STATUS_MASTER_MAINPAGE 0x39
#define REG_FIFO_STATUS1 0x3A
#define REG_FIFO_STATUS2 0x3B
#define REG_TIMESTAMP0 0x40
#define REG_TIMESTAMP1 0x41
#define REG_TIMESTAMP2 0x42
#define REG_TIMESTAMP3 0x43
#define REG_TAP_CFG0 0x56
#define REG_TAP_CFG1 0x57
#define REG_TAP_CFG2 0x58
#define REG_TAP_THS_6D 0x59
#define REG_INT_DUR2 0x5A
#define REG_WAKE_UP_THS 0x5B
#define REG_WAKE_UP_DUR 0x5C
#define REG_FREE_FALL 0x5D
#define REG_MD1_CFG 0x5E
#define REG_MD2_CFG 0x5F
#define REG_S4S_ST_CMD_CODE 0x60
#define REG_S4S_DT_REG 0x61
#define REG_I3C_BUS_AVB 0x62
#define REG_INTERNAL_FREQ_FINE 0x63
#define REG_INT_OIS 0x6F
#define REG_CTRL1_OIS 0x70
#define REG_CTRL2_OIS 0x71
#define REG_CTRL3_OIS 0x72
#define REG_X_OFS_USR 0x73
#define REG_Y_OFS_USR 0x74
#define REG_Z_OFS_USR 0x75
#define REG_FIFO_DATA_OUT_TAG 0x78
#define REG_FIFO_DATA_OUT_X_L 0x79
#define REG_FIFO_DATA_OUT_X_H 0x7A
#define REG_FIFO_DATA_OUT_Y_L 0x7B
#define REG_FIFO_DATA_OUT_Y_H 0x7C
#define REG_FIFO_DATA_OUT_Z_L 0x7D
#define REG_FIFO_DATA_OUT_Z_H 0x7E

#define IMU_SPI DT_NODELABEL(imu_spi)

// SPI master functionality
const struct device *spi_dev;

static const struct spi_cs_control spim_cs = {
    .gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(reg_imu_spi)),
    .delay = 0,
};

static const struct spi_config spi_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
                 SPI_MODE_CPOL | SPI_MODE_CPHA,
    .frequency = 4000000,
    .slave = 0,
    .cs = spim_cs,
};

void write_register(uint8_t addr, uint8_t data)
{
    // Arrange the data
    struct spi_buf tx_buf = {.buf = {addr | (1 << 7), data}, .len = 2};
    struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};

    // Write
    spi_write(spi_dev, &spi_cfg, &tx_bufs);
}

uint8_t read_register(uint8_t addr)
{
    // Set up the tx and rx buffers
    struct spi_buf tx_buf = {.buf = {addr & ~(1 << 7), 0x00}, .len = 2};
    struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};

    struct spi_buf rx_buf = {.buf = {0x00, 0x00}, .len = 2};
    struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

    // Perform the read
    spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);

    return ((uint8_t *)rx_buf.buf)[1];
}

/**
 * Initialize the IMU
 */
void imu_init()
{
    spi_dev = DEVICE_DT_GET(IMU_SPI);

    if (!device_is_ready(spi_dev))
    {
        printk("SPI master device not ready!\n");
    }
    if (!device_is_ready(spim_cs.gpio.port))
    {
        printk("SPI master chip select device not ready!\n");
    }

    // Configure the IMU to begin taking measurements

    // Set the accelerometer to 52Hz, 8g scale
    write_register(REG_CTRL1_XL, (0x3 << 4) | (0x3 << 2));

    // Set gyro to 52Hz, 250dps scale
    write_register(REG_CTRL2_G, (0x3 << 4) | (0x0 << 2));
}

/**
 * Get the current data values from the IMU
 *
 * @param rdata A pointer to a data structure to store the results in
 */
void imu_get(imu_data_t *rdata)
{
    rdata->temp = ((uint16_t)read_register(REG_OUT_TEMP_H) << 8) | read_register(REG_OUT_TEMP_L);

    rdata->accel_x = ((uint16_t)read_register(REG_OUTX_H_A) << 8) | read_register(REG_OUTX_L_A);
    rdata->accel_y = ((uint16_t)read_register(REG_OUTY_H_A) << 8) | read_register(REG_OUTY_L_A);
    rdata->accel_z = ((uint16_t)read_register(REG_OUTZ_H_A) << 8) | read_register(REG_OUTZ_L_A);

    rdata->gyro_x = ((uint16_t)read_register(REG_OUTX_H_G) << 8) | read_register(REG_OUTX_L_G);
    rdata->gyro_y = ((uint16_t)read_register(REG_OUTY_H_G) << 8) | read_register(REG_OUTY_L_G);
    rdata->gyro_z = ((uint16_t)read_register(REG_OUTZ_H_G) << 8) | read_register(REG_OUTZ_L_G);
}
