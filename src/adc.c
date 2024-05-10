#include "adc.h"

#include <nrfx_saadc.h>

#define ANALOG_INPUT_TO_SAADC_AIN(x) ((x) + 1)

#define ANALOG_INPUT_A0 0
#define ANALOG_INPUT_A1 1
#define ANALOG_INPUT_A2 2
#define ANALOG_INPUT_A3 3
#define ANALOG_INPUT_A4 4
#define ANALOG_INPUT_A5 5

#define CH0_AIN ANALOG_INPUT_TO_SAADC_AIN(ANALOG_INPUT_A0)
#define CH1_AIN ANALOG_INPUT_TO_SAADC_AIN(ANALOG_INPUT_A1)
#define CH2_AIN ANALOG_INPUT_TO_SAADC_AIN(ANALOG_INPUT_A2)
#define CH3_AIN ANALOG_INPUT_TO_SAADC_AIN(ANALOG_INPUT_A3)
#define CH4_AIN ANALOG_INPUT_TO_SAADC_AIN(ANALOG_INPUT_A4)
#define CH5_AIN ANALOG_INPUT_TO_SAADC_AIN(ANALOG_INPUT_A5)

static const nrfx_saadc_channel_t m_multiple_channels[] = {
    NRFX_SAADC_DEFAULT_CHANNEL_SE(ANALOG_INPUT_TO_SAADC_AIN(THUMB_SENSOR), 0),
    NRFX_SAADC_DEFAULT_CHANNEL_SE(ANALOG_INPUT_TO_SAADC_AIN(INDEX_SENSOR), 1),
    NRFX_SAADC_DEFAULT_CHANNEL_SE(ANALOG_INPUT_TO_SAADC_AIN(MIDDLE_SENSOR), 2),
    NRFX_SAADC_DEFAULT_CHANNEL_SE(ANALOG_INPUT_TO_SAADC_AIN(RING_SENSOR), 3),
    NRFX_SAADC_DEFAULT_CHANNEL_SE(ANALOG_INPUT_TO_SAADC_AIN(PALM1_SENSOR), 4),
    NRFX_SAADC_DEFAULT_CHANNEL_SE(ANALOG_INPUT_TO_SAADC_AIN(PALM2_SENSOR), 5),
};

#define CHANNEL_COUNT NRFX_ARRAY_SIZE(m_multiple_channels)

// #define RESOLUTION NRF_SAADC_RESOLUTION_8BIT
#define RESOLUTION NRF_SAADC_RESOLUTION_14BIT

#if (NRF_SAADC_8BIT_SAMPLE_WIDTH == 8) && (RESOLUTION == NRF_SAADC_RESOLUTION_8BIT)
static uint8_t m_samples_buffer[CHANNEL_COUNT];
#else
static uint16_t m_samples_buffer[CHANNEL_COUNT];
#endif

static bool m_saadc_ready = false;

/**
 * Initialize the ADC
 */
void adc_init()
{
    nrfx_err_t status;
    (void)status;

    m_saadc_ready = false;

    status = nrfx_saadc_init(NRFX_SAADC_DEFAULT_CONFIG_IRQ_PRIORITY);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_channels_config(m_multiple_channels, CHANNEL_COUNT);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    uint32_t channels_mask = nrfx_saadc_channels_configured_get();
    status = nrfx_saadc_simple_mode_set(channels_mask,
                                        RESOLUTION,
                                        NRF_SAADC_OVERSAMPLE_DISABLED,
                                        NULL);

    status = nrfx_saadc_buffer_set(m_samples_buffer, CHANNEL_COUNT);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_offset_calibrate(NULL);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    m_saadc_ready = true;
}

/**
 * Read the value of a force sensor at a given position
 *
 * @param pos The position of the sensor to read from
 */
void read_force_sensors(force_data_t *data)
{
    nrfx_err_t status;

    NRFX_ASSERT(m_saadc_ready == true);

    status = nrfx_saadc_mode_trigger();
    NRFX_ASSERT(status == NRFX_SUCCESS);

    for (int i = 0; i < CHANNEL_COUNT; i++)
    {
        uint16_t sample = NRFX_SAADC_SAMPLE_GET(RESOLUTION, m_samples_buffer, i);

        if (i < sizeof(force_data_t) / sizeof(uint16_t))
            ((uint16_t *)data)[i] = sample;
    }
}
