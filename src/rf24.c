/**
 * @file rf24.c
 *
 * @brief nRF24L01 registers related.
 *
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Lucas Haug <lucas.haug@thunderatz.org>
 * @author Daniel Nery <daniel.nery@thunderatz.org>
 *
 * @date 10/2019
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "rf24.h"
#include "rf24_platform.h"

/*****************************************
 * Private Constants
 *****************************************/

/**
 * @brief Defalt device values.
 */
#define DEFAULT_SPI_TIMEOUT_MS 1000U
#define DEFAULT_PAYLOAD_SIZE 32U
#define DEFAULT_ADDRESS_SIZE 5U
#define DEFAULT_CHANNEL_MHZ 76U

#define MAX_RETRANSMISSIONS 0xFU

/**
 * @brief Number of retransmissions delay steps.
 *
 * @note Each delay step is 250us.
 *
 * @note The delay can be calculated by (steps + 1) * 250
 *       in microseconds.
 *
 * @note This delay is defined from end of transmission to start
 *       of next transmission.
 */
#define NUM_OF_RETRANSMISSIONS_DELAY_STEPS 5U

/**
 * @brief Width of operationg frequency. RF module con operate
 *        on frequencies from 2.400GHz to 2.525GHz.
 */
#define OPERATING_FREQUENCY_WIDTH_MHZ 125U

/**
 * @brief Max number of pipes available.
 */
#define MAX_NUM_OF_PIPES 6

/**
 * @brief Error value for status register.
 *
 * @note Bit 7 only allows 0, so 0xFF represents an error value.
 */
#define STATUS_REG_ERROR_VALUE 0xFF

/**
 * @brief Error value for channel.
 *
 * @note The max channel value is 125, so 0xFF represents an error value.
 */
#define CHANNEL_ERROR_VALUE 0xFF

/*****************************************
 * Private Macros
 *****************************************/

/**
 * @brief Get bit value
 */
#define _BV(num) (1 << (num))

/*****************************************
 * Private Variables
 *****************************************/

static const uint8_t m_child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};

static const uint8_t m_child_payload_size[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};

static const uint8_t m_child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};

/**
 *
 * The driver will delay for this duration when stopListening() is called
 *
 * When responding to payloads, faster devices like ARM(RPi) are much faster than Arduino:
 * 1. Arduino sends data to RPi, switches to RX mode
 * 2. The RPi receives the data, switches to TX mode and sends before the Arduino radio is in RX mode
 * 3. If AutoACK is disabled, this can be set as low as 0. If AA/ESB enabled, set to 100uS minimum on RPi
 *
 * @warning If set to 0, ensure 130uS delay after stopListening() and before any sends
 */
static uint32_t m_tx_delay = 250;

/*****************************************
 * Public Functions Bodies Definitions
 *****************************************/

rf24_status_t rf24_get_default_config(rf24_dev_t* p_dev) {
    p_dev->platform_setup.spi_timeout = DEFAULT_SPI_TIMEOUT_MS;
    p_dev->payload_size = DEFAULT_PAYLOAD_SIZE;
    p_dev->addr_width = DEFAULT_ADDRESS_SIZE;
    p_dev->datarate = RF24_1MBPS;
    p_dev->channel = DEFAULT_CHANNEL_MHZ;

    for (uint8_t i = 0; i < RF24_ADDRESS_MAX_SIZE; i++) {
        p_dev->pipe0_reading_address[i] = 0;
    }

    return RF24_SUCCESS;
}

rf24_status_t rf24_init(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_rf_setup_t rf_setup_reg;

    if (dev_status == RF24_SUCCESS) {
        rf24_platform_init(&(p_dev->platform_setup));
    }

    rf24_delay(5);

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, 0x0C);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_set_retries(p_dev, NUM_OF_RETRANSMISSIONS_DELAY_STEPS, MAX_RETRANSMISSIONS);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_set_datarate(p_dev, p_dev->datarate);
    }

    if (dev_status == RF24_SUCCESS) {
        platform_status =
            rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &(rf_setup_reg.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_feature_t reg_feature = {0x00};
        reg_feature.en_dyn_ack = 1;
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_FEATURE, reg_feature.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_dynpd_t reg_dynpd = {0x00};
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_DYNPD, reg_dynpd.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_set_channel(p_dev, p_dev->channel);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_flush_rx(p_dev);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_flush_tx(p_dev);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_power_up(p_dev);
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_config_t reg_config;
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, &(reg_config.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        if (dev_status == RF24_SUCCESS) {
            reg_config.prim_rx = 0;
            rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, reg_config.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        }
    }

    if (dev_status == RF24_SUCCESS) {
        // If setup is 0x00 or 0xFF then there was no response from module
        if ((rf_setup_reg.value == 0x00) || (rf_setup_reg.value == 0xFF)) {
            dev_status = RF24_UNKNOWN_ERROR;
        }
    }

    return dev_status;
}

rf24_status_t rf24_power_up(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;
    nrf24l01_reg_config_t reg_config;

    platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, &(reg_config.value));
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    if (dev_status == RF24_SUCCESS) {
        if (reg_config.pwr_up == 1) {
            return dev_status;
        }
    }

    reg_config.pwr_up = 1;
    rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, reg_config.value);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    rf24_delay(5);

    return dev_status;
}

rf24_status_t rf24_power_down(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;
    nrf24l01_reg_config_t reg_config;

    platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, &(reg_config.value));
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    if (dev_status == RF24_SUCCESS) {
        if (reg_config.pwr_up == 0) {
            return dev_status;
        }
    }

    rf24_platform_disable(&(p_dev->platform_setup));
    reg_config.pwr_up = 0;
    platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, reg_config.value);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    return dev_status;
}

rf24_status_t rf24_set_channel(rf24_dev_t* p_dev, uint8_t ch) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    ch = ch > OPERATING_FREQUENCY_WIDTH_MHZ ? OPERATING_FREQUENCY_WIDTH_MHZ : ch;
    platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_CH, ch);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    if (dev_status == RF24_SUCCESS) {
        p_dev->channel = ch;
    }

    return dev_status;
}

uint8_t rf24_get_channel(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;
    nrf24l01_reg_rf_ch_t reg;

    platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_CH, &(reg.value));
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    if (dev_status != RF24_SUCCESS) {
        return CHANNEL_ERROR_VALUE;
    }

    return reg.rf_ch;
}

rf24_status_t rf24_set_retries(rf24_dev_t* p_dev, uint8_t delay_steps, uint8_t rt_count) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_setup_retr_t reg;
    reg.ard = delay_steps;
    reg.arc = rt_count;

    platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_SETUP_RETR, reg.value);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    return dev_status;
}

rf24_status_t rf24_set_datarate(rf24_dev_t* p_dev, rf24_datarate_t datarate) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    p_dev->datarate = datarate;
    nrf24l01_reg_rf_setup_t reg_rf_setup;

    if (dev_status == RF24_SUCCESS) {
        platform_status =
            rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &(reg_rf_setup.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    //! @todo Use m_tx_delay

    switch (datarate) {
        case RF24_1MBPS: {
            reg_rf_setup.rf_dr_low = 0;
            reg_rf_setup.rf_dr_high = 0;
            break;
        }

        case RF24_2MBPS: {
            reg_rf_setup.rf_dr_low = 0;
            reg_rf_setup.rf_dr_high = 1;
            break;
        }

        case RF24_250KBPS: {
            reg_rf_setup.rf_dr_low = 1;
            reg_rf_setup.rf_dr_high = 0;
            break;
        }
    }

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, reg_rf_setup.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        uint8_t temp_reg;
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &temp_reg);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        if (dev_status == RF24_SUCCESS) {
            if (temp_reg != reg_rf_setup.value) {
                dev_status = RF24_UNKNOWN_ERROR;
            }
        }
    }

    return dev_status;
}

rf24_status_t rf24_set_output_power(rf24_dev_t* p_dev, rf24_output_power_t output_power) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_rf_setup_t reg_rf_setup;

    if (dev_status == RF24_SUCCESS) {
        platform_status =
            rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &(reg_rf_setup.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        reg_rf_setup.rf_pwr = (uint8_t) output_power;
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, reg_rf_setup.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        uint8_t temp_reg;
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &temp_reg);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        if (dev_status == RF24_SUCCESS) {
            if (temp_reg != reg_rf_setup.value) {
                dev_status = RF24_UNKNOWN_ERROR;
            }
        }
    }

    return dev_status;
}

rf24_status_t rf24_flush_rx(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    platform_status = rf24_platform_send_command(&(p_dev->platform_setup), NRF24L01_COMM_FLUSH_RX);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    return dev_status;
}

rf24_status_t rf24_flush_tx(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    platform_status = rf24_platform_send_command(&(p_dev->platform_setup), NRF24L01_COMM_FLUSH_TX);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    return dev_status;
}

rf24_status_t rf24_open_writing_pipe(rf24_dev_t* p_dev, uint8_t* address) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    platform_status = rf24_platform_write_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P0, address,
                                                   p_dev->addr_width);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    platform_status = rf24_platform_write_register(&(p_dev->platform_setup), NRF24L01_REG_TX_ADDR, address,
                                                   p_dev->addr_width);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_RX_PW_P0, p_dev->payload_size);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    return dev_status;
}

rf24_status_t rf24_open_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number, uint8_t* address) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (pipe_number == 0) {
        memcpy(p_dev->pipe0_reading_address, address, p_dev->addr_width);
    }

    if (pipe_number < MAX_NUM_OF_PIPES) {
        // For pipes 2-5, only write the LSB
        if (pipe_number <= 1) {
            platform_status = rf24_platform_write_register(&(p_dev->platform_setup), m_child_pipe[pipe_number], address,
                                                           p_dev->addr_width);
        } else {
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), m_child_pipe[pipe_number], address[0]);
        }

        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), m_child_payload_size[pipe_number],
                                                   p_dev->payload_size);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    } else {
        dev_status = RF24_INVALID_PARAMETERS;
    }

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, it was thought it would make the calling code
    // more simple to do it this way.
    nrf24l01_reg_en_rxaddr_t reg_en_rx_addr;

    if (dev_status == RF24_SUCCESS) {
        platform_status =
            rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, &(reg_en_rx_addr.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        reg_en_rx_addr.value |= _BV(m_child_pipe_enable[pipe_number]);
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR,
                                                   reg_en_rx_addr.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    return dev_status;
}

rf24_status_t rf24_close_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_en_rxaddr_t reg_en_rx_addr;

    if (dev_status == RF24_SUCCESS) {
        platform_status =
            rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, &(reg_en_rx_addr.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        reg_en_rx_addr.value &= (~_BV(m_child_pipe_enable[pipe_number]));
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR,
                                                   reg_en_rx_addr.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    return dev_status;
}

rf24_status_t rf24_start_listening(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;

    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;
    nrf24l01_reg_config_t reg_config;
    nrf24l01_reg_status_t reg_status;

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, &(reg_config.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        reg_config.value |= _BV(PRIM_RX);
        reg_status.value = (_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

        if (dev_status == RF24_SUCCESS) {
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, reg_config.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, reg_status.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        }
    }

    if (dev_status == RF24_SUCCESS) {
        if (p_dev->pipe0_reading_address[0] > 0) {
            platform_status = rf24_platform_write_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P0,
                                                           p_dev->pipe0_reading_address, p_dev->addr_width);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        } else {
            dev_status = rf24_close_reading_pipe(p_dev, 0);
        }
    }

    // Flush buffers
    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_flush_rx(p_dev);
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_feature_t reg_feature;
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_FEATURE, &(reg_feature.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        if (dev_status == RF24_SUCCESS) {
            if (reg_feature.en_ack_pay) {
                dev_status = rf24_flush_tx(p_dev);
            }
        }
    }

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_enable(&(p_dev->platform_setup));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    return dev_status;
}

rf24_status_t rf24_stop_listening(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    rf24_platform_disable(&(p_dev->platform_setup));

    rf24_delay(m_tx_delay);

    dev_status = rf24_flush_rx(p_dev);

    nrf24l01_reg_feature_t reg_feature;

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_FEATURE, &(reg_feature.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (reg_feature.en_ack_pay) {
        rf24_delay(m_tx_delay);  // 250

        if (dev_status == RF24_SUCCESS) {
            dev_status = rf24_flush_tx(p_dev);
        }
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_config_t reg_config;
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, &(reg_config.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        if (dev_status == RF24_SUCCESS) {
            reg_config.value &= (~_BV(PRIM_RX));
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, reg_config.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        }
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_en_rxaddr_t reg_en_rx_addr;
        platform_status =
            rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, &(reg_en_rx_addr.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        reg_en_rx_addr.value |= _BV(m_child_pipe_enable[0]);

        if (dev_status == RF24_SUCCESS) {
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR,
                                                       reg_en_rx_addr.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        }
    }

    return dev_status;
}

rf24_status_t rf24_available(rf24_dev_t* p_dev, uint8_t* pipe_number) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;
    nrf24l01_reg_fifo_status_t reg_fifo_status;

    if (dev_status == RF24_SUCCESS) {
        platform_status =
            rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_FIFO_STATUS, &(reg_fifo_status.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (reg_fifo_status.rx_empty) {
        dev_status = RF24_RX_FIFO_EMPTY;
    }

    if (dev_status == RF24_SUCCESS) {
        if (pipe_number) {
            nrf24l01_reg_status_t reg_status = rf24_get_status(p_dev);
            (*pipe_number) = (uint8_t) reg_status.rx_p_no;
        }
    }

    return dev_status;
}

rf24_status_t rf24_read(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    if (len < p_dev->payload_size) {
        return RF24_BUFFER_TOO_SMALL;
    }

    nrf24l01_reg_status_t status_reg = rf24_get_status(p_dev);

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_read_payload(&(p_dev->platform_setup), buff, len);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    // Clears data ready interruption bit. But data ready utility still not implemented.
    if (dev_status == RF24_SUCCESS) {
        status_reg.rx_dr = 1;
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, status_reg.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_INTERRUPT_NOT_CLEARED);
    }

    return dev_status;
}

rf24_status_t rf24_write(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len, bool enable_auto_ack) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_status_t status_reg = rf24_get_status(p_dev);

    if (status_reg.tx_full) {
        return RF24_TX_FIFO_FULL;
    }

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_write_payload(&(p_dev->platform_setup), buff, len, enable_auto_ack);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        rf24_platform_enable(&(p_dev->platform_setup));
    }

    do {
        status_reg = rf24_get_status(p_dev);
    } while (!status_reg.tx_ds && !status_reg.max_rt);

    if (dev_status == RF24_SUCCESS) {
        rf24_platform_disable(&(p_dev->platform_setup));
    }

    // Max retries exceeded
    if (status_reg.max_rt) {
        dev_status = RF24_MAX_RETRANSMIT;

        status_reg.max_rt = 1;  // Datasheet says to write 1 to clear the interruption bit.
        rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, status_reg.value);
        rf24_flush_tx(p_dev);  // Only going to be 1 packet in the FIFO at a time using this method, so just flush.

        return dev_status;
    }

    status_reg.tx_ds = 1;
    platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, status_reg.value);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_INTERRUPT_NOT_CLEARED);

    return dev_status;
}

rf24_status_t rf24_write_continuously(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_status_t status_reg = rf24_get_status(p_dev);

    if (status_reg.tx_full) {
        return RF24_TX_FIFO_FULL;
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_set_retries(p_dev, NUM_OF_RETRANSMISSIONS_DELAY_STEPS, 0);
    }

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_write_payload(&(p_dev->platform_setup), buff, len, false);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        rf24_platform_enable(&(p_dev->platform_setup));
    }

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_send_command(&(p_dev->platform_setup), NRF24L01_COMM_REUSE_TX_PL);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    return dev_status;
}

nrf24l01_reg_status_t rf24_get_status(rf24_dev_t* p_dev) {
    nrf24l01_reg_status_t status_reg;
    rf24_platform_status_t platform_status = rf24_platform_get_status(&(p_dev->platform_setup), &status_reg);

    if (platform_status != RF24_PLATFORM_SUCCESS) {
        status_reg.value = STATUS_REG_ERROR_VALUE;
    }

    return status_reg;
}

rf24_status_t rf24_set_irq_configuration(rf24_dev_t* p_dev, rf24_irq_t irq_config) {
    nrf24l01_reg_config_t config_reg;

    rf24_platform_status_t platform_status =
        rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, &(config_reg.value));

    if (platform_status == RF24_PLATFORM_SUCCESS) {
        config_reg.mask_max_rt = irq_config.max_retransmits;
        config_reg.mask_tx_ds = irq_config.tx_data_sent;
        config_reg.mask_rx_dr = irq_config.rx_data_ready;

        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, config_reg.value);
    }

    return platform_status;
}

rf24_irq_t rf24_irq_callback(rf24_dev_t* p_dev) {
    rf24_irq_t irq_values = {
        .tx_data_sent = 0,
        .rx_data_ready = 0,
        .max_retransmits = 0,
    };

    nrf24l01_reg_status_t status_reg;
    rf24_platform_status_t platform_status = rf24_platform_get_status(&(p_dev->platform_setup), &status_reg);

    if (platform_status == RF24_PLATFORM_SUCCESS) {
        irq_values.tx_data_sent = status_reg.tx_ds;
        irq_values.rx_data_ready = status_reg.rx_dr;
        irq_values.max_retransmits = status_reg.max_rt;

        // Resets interruptions flags values
        rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, status_reg.value);
    }

    return irq_values;
}

__weak rf24_status_t rf24_delay(uint32_t ms);
