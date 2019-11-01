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

#include "rf24.h"
#include "rf24_platform.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*****************************************
 * Private Macros
 *****************************************/

#ifdef DEBUG
#define PRINTF printf
#else
#define PRINTF(...)
#endif

#define _BS(bit) (1 << (bit))

/*****************************************
 * Private Variables
 *****************************************/

static const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};

static const uint8_t child_payload_size[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};

static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};

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
static uint32_t txDelay = 250; // q merda é essa?? eu n sei

/*****************************************
 * Public Functions Bodies Definitions
 *****************************************/

rf24_dev_t rf24_get_default_config(void) {
    return (rf24_dev_t) {
               .platform_setup = {
                   .spi_timeout = 1000,
               },
               .payload_size = 32,
               .addr_width = 5,
               .datarate = RF24_1MBPS,
               .channel = 76,
               .pipe0_reading_address = {0, 0, 0, 0, 0},
    };
}

rf24_status_t rf24_init(rf24_dev_t* p_dev) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_rf_setup_t rf_setup_reg;

    if (dev_status == RF24_SUCCESS) {
        rf24_platform_init(&(p_dev->platform_setup));
    }

    HAL_Delay(5);

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, 0x0C);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_set_retries(p_dev, 5, 15);
    }

    if (dev_status == RF24_SUCCESS) {
        dev_status = rf24_set_datarate(p_dev, p_dev->datarate);
    }

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &(rf_setup_reg.value));
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
        if ((rf_setup_reg.value == 0) || (rf_setup_reg.value == 0xFF)){
            dev_status = RF24_UNKNOWN_ERRO;
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

    //! @todo Dont use HAL_Delay()
    HAL_Delay(5);

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

    ch = ch > 125 ? 125 : ch; // TODO define
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
        return 0xFF;
    }

    return reg.rf_ch;
}

rf24_status_t rf24_set_retries(rf24_dev_t* p_dev, uint8_t delay, uint8_t count) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_setup_retr_t reg;
    reg.ard = delay;
    reg.arc = count;

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
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &(reg_rf_setup.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    //! @todo Coisas com txDelay (??)

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
                dev_status = RF24_UNKNOWN_ERRO;
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
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_RF_SETUP, &(reg_rf_setup.value));
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
                dev_status = RF24_UNKNOWN_ERRO;
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

    platform_status = rf24_platform_write_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P0, address, p_dev->addr_width);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

    platform_status = rf24_platform_write_register(&(p_dev->platform_setup), NRF24L01_REG_TX_ADDR, address, p_dev->addr_width);
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

    if (pipe_number <= 5) {
        // For pipes 2-5, only write the LSB
        if (pipe_number <= 1) {
            platform_status = rf24_platform_write_register(&(p_dev->platform_setup), child_pipe[pipe_number], address, p_dev->addr_width);
        } else {
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), child_pipe[pipe_number], address[0]);
        }
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), child_payload_size[pipe_number], p_dev->payload_size);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    } else {
        dev_status = RF24_INVALID_PARAMETERS;
    }

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, I thought it would make the calling code
    // more simple to do it this way.
    nrf24l01_reg_en_rxaddr_t reg_en_rx_addr;
    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, &(reg_en_rx_addr.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        reg_en_rx_addr.value |= _BS(child_pipe_enable[pipe_number]);
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, reg_en_rx_addr.value);
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    return dev_status;
}

rf24_status_t rf24_close_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number) {
    rf24_status_t dev_status = RF24_SUCCESS;
    rf24_platform_status_t platform_status = RF24_PLATFORM_SUCCESS;

    nrf24l01_reg_en_rxaddr_t reg_en_rx_addr;

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, &(reg_en_rx_addr.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (dev_status == RF24_SUCCESS) {
        reg_en_rx_addr.value &= (~_BS(child_pipe_enable[pipe_number]));
        platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, reg_en_rx_addr.value);
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

        reg_config.value |= _BS(PRIM_RX);
        reg_status.value = (_BS(RX_DR) | _BS(TX_DS) | _BS(MAX_RT));

        if (dev_status == RF24_SUCCESS) {
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, reg_config.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, reg_status.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        }
    }

    if (dev_status == RF24_SUCCESS) {
        if (p_dev->pipe0_reading_address[0] > 0) {
            platform_status = rf24_platform_write_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P0, p_dev->pipe0_reading_address, p_dev->addr_width);
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

    //! @todo do not use HAL_Delay
    HAL_Delay(txDelay);

    dev_status = rf24_flush_rx(p_dev);

    nrf24l01_reg_feature_t reg_feature;

    if (dev_status == RF24_SUCCESS) {
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_FEATURE, &(reg_feature.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
    }

    if (reg_feature.en_ack_pay) {
        //! @todo do not use HAL_Delay
        HAL_Delay(txDelay);  // 250

        if (dev_status == RF24_SUCCESS) {
            dev_status = rf24_flush_tx(p_dev);
        }
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_config_t reg_config;
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, &(reg_config.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);

        if (dev_status == RF24_SUCCESS) {
            reg_config.value &= (~_BS(PRIM_RX));
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_CONFIG, reg_config.value);
            dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        }
    }

    if (dev_status == RF24_SUCCESS) {
        nrf24l01_reg_en_rxaddr_t reg_en_rx_addr;
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, &(reg_en_rx_addr.value));
        dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_ERROR_CONTROL_INTERFACE);
        reg_en_rx_addr.value |= _BS(child_pipe_enable[0]);

        if (dev_status == RF24_SUCCESS) {
            platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_EN_RXADDR, reg_en_rx_addr.value);
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
        platform_status = rf24_platform_read_reg8(&(p_dev->platform_setup), NRF24L01_REG_FIFO_STATUS, &(reg_fifo_status.value));
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

    // Limpando a interrupt de data ready, porém não usamos ela em nenhum lugar ainda
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

        status_reg.max_rt = 1; // O datasheet manda escerever 1 no bit para limpar a interrupcao
        rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, status_reg.value);
        rf24_flush_tx(p_dev); // Only going to be 1 packet int the FIFO at a time using this method, so just flush

        return dev_status;
    }

    status_reg.tx_ds = 1;
    platform_status = rf24_platform_write_reg8(&(p_dev->platform_setup), NRF24L01_REG_STATUS, status_reg.value);
    dev_status = (platform_status == RF24_PLATFORM_SUCCESS) ? (RF24_SUCCESS) : (RF24_INTERRUPT_NOT_CLEARED);


    return dev_status;
}

nrf24l01_reg_status_t rf24_get_status(rf24_dev_t* p_dev) {
    nrf24l01_reg_status_t status_reg;
    rf24_platform_status_t platform_status = rf24_platform_get_status(&(p_dev->platform_setup), &status_reg);

    if (platform_status != RF24_PLATFORM_SUCCESS) {
        status_reg.value = 0xFF;
    }

    return status_reg;  // Bit 7 only allows 0, so 0xFF represents an erro value
}

#ifdef DEBUG

uint8_t rf24_read_reg8_debug (rf24_dev_t* p_dev, nrf24l01_registers_t reg) {
    uint8_t value;
    rf24_platform_read_reg8(&(p_dev->platform_setup), reg, &value);
    return value;
}

void rf24_dump_registers(rf24_dev_t* p_dev) {
    nrf24l01_reg_config_t reg_config = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_CONFIG)};
    nrf24l01_reg_en_aa_t reg_en_aa = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_EN_AA)};
    nrf24l01_reg_en_rxaddr_t reg_en_rxaddr = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_EN_RXADDR)};
    nrf24l01_reg_setup_aw_t reg_setup_aw = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_SETUP_AW)};
    nrf24l01_reg_setup_retr_t reg_setup_retr = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_SETUP_RETR)};
    nrf24l01_reg_rf_ch_t reg_rf_ch = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RF_CH)};
    nrf24l01_reg_rf_setup_t reg_rf_setup = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RF_SETUP)};
    nrf24l01_reg_status_t reg_status = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_STATUS)};
    nrf24l01_reg_observe_tx_t reg_observe_tx = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_OBSERVE_TX)};
    nrf24l01_reg_rpd_t reg_rpd = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RPD)};

    nrf24l01_reg_5byte_addr_t reg_rx_addr_p0;
    nrf24l01_reg_5byte_addr_t reg_rx_addr_p1;
    nrf24l01_reg_5byte_addr_t reg_tx_addr;
    rf24_platform_read_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P0, reg_rx_addr_p0.value, p_dev->addr_width);
    rf24_platform_read_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P1, reg_rx_addr_p1.value, p_dev->addr_width);
    rf24_platform_read_register(&(p_dev->platform_setup), NRF24L01_REG_TX_ADDR, reg_tx_addr.value, p_dev->addr_width);

    nrf24l01_reg_1byte_addr_t reg_rx_addr_p2 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_ADDR_P2)};
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p3 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_ADDR_P3)};
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p4 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_ADDR_P4)};
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p5 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_ADDR_P5)};
    nrf24l01_reg_rx_pw_p0_t reg_rx_pw_p0 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_PW_P0)};
    nrf24l01_reg_rx_pw_p1_t reg_rx_pw_p1 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_PW_P1)};
    nrf24l01_reg_rx_pw_p2_t reg_rx_pw_p2 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_PW_P2)};
    nrf24l01_reg_rx_pw_p3_t reg_rx_pw_p3 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_PW_P3)};
    nrf24l01_reg_rx_pw_p4_t reg_rx_pw_p4 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_PW_P4)};
    nrf24l01_reg_rx_pw_p5_t reg_rx_pw_p5 = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_RX_PW_P5)};
    nrf24l01_reg_fifo_status_t reg_fifo_status = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_FIFO_STATUS)};
    nrf24l01_reg_dynpd_t reg_dynpd = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_DYNPD)};
    nrf24l01_reg_feature_t reg_feature = {rf24_read_reg8_debug(p_dev, NRF24L01_REG_FEATURE)};

    PRINTF("=== REGISTER DUMP\r\n");
    PRINTF(
        "[00] CONFIG      = 0x%02X | MASK_RX_DR=%d  MASK_TX_DS=%d  MASK_MAX_RT=%d EN_CRC=%d      CRCO=%d        PWR_UP=%d      PRIM_RX=%d\r\n",
        reg_config.value, reg_config.mask_rx_dr, reg_config.mask_tx_ds, reg_config.mask_max_rt,
        reg_config.en_crc, reg_config.crco, reg_config.pwr_up, reg_config.prim_rx);
    PRINTF(
        "[01] EN_AA       = 0x%02X | ENAA_P5=%d     ENAA_P4=%d     ENAA_P3=%d     ENAA_P2=%d     ENAA_P1=%d     ENAA_P0=%d\r\n",
        reg_en_aa.value, reg_en_aa.enaa_p5, reg_en_aa.enaa_p4, reg_en_aa.enaa_p3,
        reg_en_aa.enaa_p2, reg_en_aa.enaa_p1, reg_en_aa.enaa_p0);
    PRINTF(
        "[02] EN_RXADDR   = 0x%02X | ERX_P5=%d      ERX_P4=%d      ERX_P3=%d      ERX_P2=%d      ERX_P1=%d      ERX_P0=%d\r\n",
        reg_en_rxaddr.value, reg_en_rxaddr.erx_p5, reg_en_rxaddr.erx_p4, reg_en_rxaddr.erx_p3,
        reg_en_rxaddr.erx_p2, reg_en_rxaddr.erx_p1, reg_en_rxaddr.erx_p0);
    PRINTF("[03] SETUP_AW    = 0x%02X | AW=%d (%s)\r\n", reg_setup_aw.value, reg_setup_aw.aw,
           (reg_setup_aw.aw == 0b00) ? "Illegal" :
           (reg_setup_aw.aw == 0b01) ? "3 bytes" :
           (reg_setup_aw.aw == 0b10) ? "4 bytes" :
           (reg_setup_aw.aw == 0b11) ? "5 bytes" : "???????");
    PRINTF("[04] SETUP_RETR  = 0x%02X | ARD=%d (%d us)  ARC=%d (retransmits)\r\n", reg_setup_retr.value,
           reg_setup_retr.ard, 250 * (1 + reg_setup_retr.ard), reg_setup_retr.arc);
    PRINTF("[05] RF_CH       = 0x%02X | RF_CH=%d (%d MHz)\r\n", reg_rf_ch.value,
           reg_rf_ch.rf_ch, reg_rf_ch.rf_ch + 2400);
    PRINTF("[06] RF_SETUP    = 0x%02X | CONT_WAVE=%d   RF_DR=%d%d (%s) PLL_LOCK=%d  RF_PWR=%d (%sdBm)\r\n",
           reg_rf_setup.value, reg_rf_setup.cont_wave, reg_rf_setup.rf_dr_low, reg_rf_setup.rf_dr_high,
           (reg_rf_setup.rf_dr_low == 0 && reg_rf_setup.rf_dr_high == 0) ? "1Mbps" :
           (reg_rf_setup.rf_dr_low == 0 && reg_rf_setup.rf_dr_high == 1) ? "2Mbps" :
           (reg_rf_setup.rf_dr_low == 1 && reg_rf_setup.rf_dr_high == 0) ? "250kbps" : "?????",
           reg_rf_setup.pll_lock, reg_rf_setup.rf_pwr,
           (reg_rf_setup.rf_pwr == 0b00) ? "-18" :
           (reg_rf_setup.rf_pwr == 0b01) ? "-12" :
           (reg_rf_setup.rf_pwr == 0b10) ? " -6" :
           (reg_rf_setup.rf_pwr == 0b11) ? "  0" : "???????");
    PRINTF("[07] STATUS      = 0x%02X | RX_DR=%d  TX_DS=%d  MAX_RT=%d  RX_P_NO=%d  TX_FULL=%d\r\n", reg_status.value,
           reg_status.rx_dr, reg_status.tx_ds, reg_status.max_rt, reg_status.rx_p_no,
           reg_status.tx_full);
    PRINTF("[08] OBSERVE_TX  = 0x%02X | PLOS_CNT=%d    ARC_CNT=%d\r\n", reg_observe_tx.value,
           reg_observe_tx.plos_cnt, reg_observe_tx.arc_cnt);
    PRINTF("[09] RPD         = 0x%02X | RPD=%d\r\n", reg_rpd.value, reg_rpd.rpd);
    PRINTF("[0A] RX_ADDR_P0  = [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]\r\n", reg_rx_addr_p0.value[0], reg_rx_addr_p0.value[1],
           reg_rx_addr_p0.value[2], reg_rx_addr_p0.value[3], reg_rx_addr_p0.value[4]);
    PRINTF("[0B] RX_ADDR_P1  = [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]\r\n", reg_rx_addr_p1.value[0], reg_rx_addr_p1.value[1],
           reg_rx_addr_p1.value[2], reg_rx_addr_p1.value[3], reg_rx_addr_p1.value[4]);
    PRINTF("[0C] RX_ADDR_P2  = 0x%02X\r\n", reg_rx_addr_p2.value);
    PRINTF("[0D] RX_ADDR_P3  = 0x%02X\r\n", reg_rx_addr_p3.value);
    PRINTF("[0E] RX_ADDR_P4  = 0x%02X\r\n", reg_rx_addr_p4.value);
    PRINTF("[0F] RX_ADDR_P5  = 0x%02X\r\n", reg_rx_addr_p5.value);
    PRINTF("[10] TX_ADDR     = [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]\r\n", reg_tx_addr.value[0], reg_tx_addr.value[1],
           reg_tx_addr.value[2], reg_tx_addr.value[3], reg_tx_addr.value[4]);
    PRINTF("[11] RX_PW_P0    = 0x%02X\r\n", reg_rx_pw_p0.value);
    PRINTF("[12] RX_PW_P1    = 0x%02X\r\n", reg_rx_pw_p1.value);
    PRINTF("[13] RX_PW_P2    = 0x%02X\r\n", reg_rx_pw_p2.value);
    PRINTF("[14] RX_PW_P3    = 0x%02X\r\n", reg_rx_pw_p3.value);
    PRINTF("[15] RX_PW_P4    = 0x%02X\r\n", reg_rx_pw_p4.value);
    PRINTF("[16] RX_PW_P5    = 0x%02X\r\n", reg_rx_pw_p5.value);
    PRINTF("[17] FIFO_STATUS = 0x%02X | TX_REUSE=%d  TX_FULL=%d  TX_EMPTY=%d  RX_FULL=%d  RX_EMPTY=%d\r\n", reg_fifo_status.value,
           reg_fifo_status.tx_reuse, reg_fifo_status.tx_full, reg_fifo_status.tx_empty, reg_fifo_status.rx_full,
           reg_fifo_status.rx_empty);
    PRINTF("[1C] DYNPD       = 0x%02X | DPL_P5=%d  DPL_P4=%d  DPL_P3=%d  DPL_P2=%d  DPL_P1=%d  DPL_P0=%d\r\n", reg_dynpd.value,
           reg_dynpd.dpl_p5, reg_dynpd.dpl_p4, reg_dynpd.dpl_p3, reg_dynpd.dpl_p2,
           reg_dynpd.dpl_p1, reg_dynpd.dpl_p0);
    PRINTF("[1D] FEATURE     = 0x%02X | EN_DPL=%d  EN_ACK_PAY=%d  EN_DYN_ACK=%d\r\n", reg_feature.value, reg_feature.en_dpl,
           reg_feature.en_ack_pay, reg_feature.en_dyn_ack);
}

void rf24_print_status(rf24_dev_t* p_dev) {
    nrf24l01_reg_status_t reg_status = rf24_get_status(p_dev);

    PRINTF("[07] STATUS      = 0x%02X | RX_DR=%d  TX_DS=%d  MAX_RT=%d  RX_P_NO=%d  TX_FULL=%d\r\n", reg_status.value,
           reg_status.rx_dr, reg_status.tx_ds, reg_status.max_rt, reg_status.rx_p_no,
           reg_status.tx_full);
}
#endif
