#include "RF24.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#define PRINTF printf
#else
#define PRINTF(...)
#endif

uint32_t txDelay = 250;

rf24_t rf24_get_default_config(void) {
    return (rf24_t) {
               .spi_timeout = 1000,
               .payload_size = 32,
               .addr_width = 5,
               .datarate = RF24_1MBPS,
               .channel = 76,
               .pipe0_reading_address = {0, 0, 0, 0, 0},
    };
}

bool rf24_init(rf24_t* rf24) {
    rf24_end_transaction(rf24);

    HAL_Delay(5);

    rf24_write_reg8(rf24, NRF24L01_REG_CONFIG, 0x0C);

    rf24_set_retries(rf24, 5, 15);

    rf24_set_datarate(rf24, rf24->datarate);

    uint8_t setup = rf24_read_reg8(rf24, NRF24L01_REG_RF_SETUP);

    rf24_set_datarate(rf24, RF24_1MBPS);

    nrf24l01_reg_feature_t reg_feature = {0x00};
    nrf24l01_reg_dynpd_t reg_dynpd = {0x00};
    reg_feature.en_dyn_ack = 1;
    rf24_write_reg8(rf24, NRF24L01_REG_FEATURE, reg_feature.value);
    rf24_write_reg8(rf24, NRF24L01_REG_DYNPD, reg_dynpd.value);

    rf24_set_channel(rf24, rf24->channel);

    rf24_flush_rx(rf24);
    rf24_flush_tx(rf24);

    rf24_power_up(rf24);

    nrf24l01_reg_config_t reg_config = { rf24_read_reg8(rf24, NRF24L01_REG_CONFIG) };
    reg_config.prim_rx = 0;
    rf24_write_reg8(rf24, NRF24L01_REG_CONFIG, reg_config.value);
    // rf24_enable(rf24); Vou fazer no start listening

    return ((setup != 0) && (setup != 0xff));
}

void rf24_power_up(rf24_t* rf24) {
    nrf24l01_reg_config_t reg_config = { rf24_read_reg8(rf24, NRF24L01_REG_CONFIG) };

    if (reg_config.pwr_up == 1) {
        return;
    }

    reg_config.pwr_up = 1;
    rf24_write_reg8(rf24, NRF24L01_REG_CONFIG, reg_config.value);
    HAL_Delay(5);
}

void rf24_power_down(rf24_t* rf24) {
    nrf24l01_reg_config_t reg_config = { rf24_read_reg8(rf24, NRF24L01_REG_CONFIG) };

    if (reg_config.pwr_up == 0) {
        return;
    }

    rf24_disable(rf24);
    reg_config.pwr_up = 0;
    rf24_write_reg8(rf24, NRF24L01_REG_CONFIG, reg_config.value);
}

void rf24_enable(rf24_t* rf24) {
    HAL_GPIO_WritePin(rf24->ce_port, rf24->ce_pin, GPIO_PIN_SET);
}

void rf24_disable(rf24_t* rf24) {
    HAL_GPIO_WritePin(rf24->ce_port, rf24->ce_pin, GPIO_PIN_RESET);
}

void rf24_set_channel(rf24_t* rf24, uint8_t ch) {
    ch = ch > 125 ? 125 : ch; // TODO define
    rf24_write_reg8(rf24, NRF24L01_REG_RF_CH, ch);
    rf24->channel = ch;
}

uint8_t rf24_get_channel(rf24_t* rf24) {
    nrf24l01_reg_rf_ch_t reg;
    reg.value = rf24_read_reg8(rf24, NRF24L01_REG_RF_CH);
    return reg.rf_ch;
}

void rf24_set_retries(rf24_t* rf24, uint8_t delay, uint8_t count) {
    nrf24l01_reg_setup_retr_t reg;
    reg.ard = delay;
    reg.arc = count;
    rf24_write_reg8(rf24, NRF24L01_REG_SETUP_RETR, reg.value);
}

bool rf24_set_datarate(rf24_t* rf24, rf24_datarate_t datarate) {
    rf24->datarate = datarate;
    nrf24l01_reg_rf_setup_t reg_rf_setup = { rf24_read_reg8(rf24, NRF24L01_REG_RF_SETUP) };

    // TODO Coisas com txDelay (??)

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

    rf24_write_reg8(rf24, NRF24L01_REG_RF_SETUP, reg_rf_setup.value);

    uint8_t temp_reg;

    if ((temp_reg = rf24_read_reg8(rf24, NRF24L01_REG_RF_SETUP)) == reg_rf_setup.value) {
        return true;
    }

    reg_rf_setup.value = temp_reg;  // Pra que isso se não faz nada dps?
    return false;
}

bool rf24_set_output_power(rf24_t* rf24, rf24_output_power_t output_power) {
    nrf24l01_reg_rf_setup_t reg_rf_setup = { rf24_read_reg8(rf24, NRF24L01_REG_RF_SETUP) };
    reg_rf_setup.rf_pwr = (uint8_t) output_power;
    rf24_write_reg8(rf24, NRF24L01_REG_RF_SETUP, reg_rf_setup.value);

    uint8_t temp_reg;

    if ((temp_reg = rf24_read_reg8(rf24, NRF24L01_REG_RF_SETUP)) == reg_rf_setup.value) {
        return true;
    }

    return false;
}

void rf24_flush_rx(rf24_t* rf24) {
    rf24_send_command(rf24, NRF24L01_COMM_FLUSH_RX);
}

void rf24_flush_tx(rf24_t* rf24) {
    rf24_send_command(rf24, NRF24L01_COMM_FLUSH_TX);
}


void rf24_open_writing_pipe(rf24_t* p_rf24, uint8_t* address) {
    rf24_write_register(p_rf24, NRF24L01_REG_RX_ADDR_P0, address, p_rf24->addr_width);
    rf24_write_register(p_rf24, NRF24L01_REG_TX_ADDR, address, p_rf24->addr_width);
    rf24_write_reg8(p_rf24,NRF24L01_REG_RX_PW_P0, p_rf24->payload_size);
}

static const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};

static const uint8_t child_payload_size[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};

static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};

void rf24_open_reading_pipe(rf24_t* p_rf24, uint8_t pipe_number, uint8_t* address) {
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (pipe_number == 0) {
        memcpy(p_rf24->pipe0_reading_address, address, p_rf24->addr_width);
    }

    if (pipe_number <= 5) {
        // For pipes 2-5, only write the LSB
        if (pipe_number <= 1) {
            rf24_write_register(p_rf24, child_pipe[pipe_number], address, p_rf24->addr_width);
        } else {
            rf24_write_reg8(p_rf24, child_pipe[pipe_number], address[0]);
        }

        rf24_write_reg8(p_rf24, child_payload_size[pipe_number], p_rf24->payload_size);
    }

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, I thought it would make the calling code
    // more simple to do it this way.
    nrf24l01_reg_en_rxaddr_t reg_en_rx_addr = { rf24_read_reg8(p_rf24, NRF24L01_REG_EN_RXADDR) };
    reg_en_rx_addr.value |= _BS(child_pipe_enable[pipe_number]);
    rf24_write_reg8(p_rf24, NRF24L01_REG_EN_RXADDR, reg_en_rx_addr.value);
}

void rf24_close_reading_pipe(rf24_t* p_rf24, uint8_t pipe_number) {
    nrf24l01_reg_en_rxaddr_t reg_en_rx_addr = { rf24_read_reg8(p_rf24, NRF24L01_REG_EN_RXADDR) };
    reg_en_rx_addr.value &= (~_BS(child_pipe_enable[pipe_number]));
    rf24_write_reg8(p_rf24, NRF24L01_REG_EN_RXADDR, reg_en_rx_addr.value);
}

void rf24_start_listening(rf24_t* p_rf24) {
    nrf24l01_reg_config_t reg_config = { rf24_read_reg8(p_rf24, NRF24L01_REG_CONFIG) };
    nrf24l01_reg_status_t reg_status;

    reg_config.value |= _BS(PRIM_RX);
    reg_status.value = ( _BS(RX_DR) | _BS(TX_DS) | _BS(MAX_RT) );

    rf24_write_reg8(p_rf24, NRF24L01_REG_CONFIG, reg_config.value);
    rf24_write_reg8(p_rf24, NRF24L01_REG_STATUS, reg_status.value);

    //rf24_enable estava aqui, coloquei pro final da função

    if (p_rf24->pipe0_reading_address[0] > 0) {
        rf24_write_register(p_rf24, NRF24L01_REG_RX_ADDR_P0, p_rf24->pipe0_reading_address, p_rf24->addr_width);
    } else {
        rf24_close_reading_pipe(p_rf24, 0);
    }

    // Flush buffers
    rf24_flush_rx(p_rf24);
    nrf24l01_reg_feature_t reg_feature = { rf24_read_reg8(p_rf24, NRF24L01_REG_FEATURE) };
    if (reg_feature.en_ack_pay) {
        rf24_flush_tx(p_rf24);
    }

    rf24_enable(p_rf24); // isso deveria ser no final da função?
}

void rf24_stop_listening(rf24_t* p_rf24) {
    rf24_disable(p_rf24);

    HAL_Delay(txDelay);

    rf24_flush_rx(p_rf24);
    nrf24l01_reg_feature_t reg_feature = { rf24_read_reg8(p_rf24, NRF24L01_REG_FEATURE) };
    if (reg_feature.en_ack_pay) {
        HAL_Delay(txDelay); //200
        rf24_flush_tx(p_rf24);
    }

    nrf24l01_reg_config_t reg_config = { rf24_read_reg8(p_rf24, NRF24L01_REG_CONFIG) };
    reg_config.value &= (~_BS(PRIM_RX));
    rf24_write_reg8(p_rf24, NRF24L01_REG_CONFIG, reg_config.value);

    nrf24l01_reg_en_rxaddr_t reg_en_rx_addr = { rf24_read_reg8(p_rf24, NRF24L01_REG_EN_RXADDR) };
    reg_en_rx_addr.value |= _BS(child_pipe_enable[0]);
    rf24_write_reg8(p_rf24, NRF24L01_REG_EN_RXADDR, reg_en_rx_addr.value);
}

nrf24l01_reg_status_t rf24_get_status(rf24_t* p_rf24) {
    return (rf24_send_command(p_rf24, NRF24L01_COMM_NOP));
}

bool rf24_available(rf24_t* p_rf24, uint8_t* pipe_number) {
    nrf24l01_reg_fifo_status_t reg_fifo_status = { rf24_read_reg8(p_rf24, NRF24L01_REG_FIFO_STATUS) };
    if (!reg_fifo_status.rx_empty) {
        if (pipe_number) {
            nrf24l01_reg_status_t reg_status = rf24_get_status(p_rf24);
            (*pipe_number) = (uint8_t) reg_status.rx_p_no;
        }
        return true;
    }
    return false;
}

bool rf24_read(rf24_t* p_rf24, uint8_t* buff, uint8_t len) {
    nrf24l01_reg_status_t status = rf24_read_payload(p_rf24, buff, len);

    //  Limpando a interrupt de data ready, porém não usamos ela em nenhum lugar ainda
    status.rx_dr = 1;
    rf24_write_reg8(p_rf24, NRF24L01_REG_STATUS, status.value);

    return true;
}

bool rf24_write(rf24_t* p_rf24, uint8_t* buff, uint8_t len, bool enable_auto_ack) {
    nrf24l01_reg_status_t status = rf24_get_status(p_rf24);
    if (status.tx_full) {
        return false;
    }
    status = rf24_write_payload(p_rf24, buff, len, enable_auto_ack);

    rf24_enable(p_rf24);

    do {
        status = rf24_get_status(p_rf24);
    } while (!status.tx_ds && !status.max_rt);

    rf24_disable(p_rf24);

    //Max retries exceeded
    if (status.max_rt) {
        status.max_rt = 1;  // O datasheet manda escerever 1 no bit para limpar a interrupcao (muito estranho esse codigo aqui)
        rf24_write_reg8(p_rf24, NRF24L01_REG_STATUS, status.value);
        rf24_flush_tx(p_rf24); //Only going to be 1 packet int the FIFO at a time using this method, so just flush
        return false;
    }

    status.tx_ds = 1;
    rf24_write_reg8(p_rf24, NRF24L01_REG_STATUS, status.value);

    return true;
}

// read/write register functions

void rf24_begin_transaction(rf24_t* rf24) {
    HAL_GPIO_WritePin(rf24->csn_port, rf24->csn_pin, GPIO_PIN_RESET);
}

void rf24_end_transaction(rf24_t* rf24) {
    HAL_GPIO_WritePin(rf24->csn_port, rf24->csn_pin, GPIO_PIN_SET);
}

nrf24l01_reg_status_t rf24_send_command(rf24_t* rf24, nrf24l01_spi_commands_t command) {
    nrf24l01_reg_status_t status;

    rf24_begin_transaction(rf24);
    HAL_SPI_TransmitReceive(rf24->hspi, &command, &(status.value), 1, rf24->spi_timeout);
    rf24_end_transaction(rf24);

    return status;
}

nrf24l01_reg_status_t rf24_read_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buff, uint8_t len) {
    nrf24l01_reg_status_t status;

    rf24_begin_transaction(rf24);

    // Transmit command byte
    uint8_t command = NRF24L01_COMM_R_REGISTER | (NRF24L01_COMM_RW_REGISTER_MASK & reg);
    HAL_SPI_TransmitReceive(rf24->hspi, &command, &(status.value), 1, rf24->spi_timeout);

    HAL_SPI_Receive(rf24->hspi, buff, len, rf24->spi_timeout);

    rf24_end_transaction(rf24);

    return status;
}

uint8_t rf24_read_reg8(rf24_t* rf24, nrf24l01_registers_t reg) {
    uint8_t val;
    rf24_read_register(rf24, reg, &val, 1);

    return val;
}

nrf24l01_reg_status_t rf24_write_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buff, uint8_t len) {
    nrf24l01_reg_status_t status;

    rf24_begin_transaction(rf24);

    uint8_t command = NRF24L01_COMM_W_REGISTER | (NRF24L01_COMM_RW_REGISTER_MASK & reg);
    HAL_SPI_TransmitReceive(rf24->hspi, &command, &(status.value), 1, rf24->spi_timeout);

    HAL_SPI_Transmit(rf24->hspi, buff, len, rf24->spi_timeout);

    rf24_end_transaction(rf24);

    return status;
}

void rf24_write_reg8(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t value) {
    rf24_write_register(rf24, reg, &value, 1);
}

nrf24l01_reg_status_t rf24_read_payload(rf24_t* p_rf24, uint8_t* buff, uint8_t len) {
    nrf24l01_reg_status_t status;

    rf24_begin_transaction(p_rf24);

    uint8_t command = NRF24L01_COMM_R_RX_PAYLOAD;
    HAL_SPI_TransmitReceive(p_rf24->hspi, &command, &(status.value), 1, p_rf24->spi_timeout);

    HAL_SPI_Receive(p_rf24->hspi, buff, len, p_rf24->spi_timeout);

    rf24_end_transaction(p_rf24);

    return status;
}

nrf24l01_reg_status_t rf24_write_payload(rf24_t* p_rf24, uint8_t* buff, uint8_t len, bool enable_auto_ack) {
    nrf24l01_reg_status_t status;

    rf24_begin_transaction(p_rf24);

    uint8_t command = enable_auto_ack ? (NRF24L01_COMM_W_TX_PAYLOAD) : (NRF24L01_COMM_W_TX_PAYLOAD_NOACK);

    HAL_SPI_TransmitReceive(p_rf24->hspi, &command, &(status.value), 1, p_rf24->spi_timeout);

    HAL_SPI_Transmit(p_rf24->hspi, buff, len, p_rf24->spi_timeout);

    rf24_end_transaction(p_rf24);

    return status;
}

#ifdef DEBUG
void rf24_dump_registers(rf24_t* rf24) {
    nrf24l01_reg_config_t reg_config = { rf24_read_reg8(rf24, NRF24L01_REG_CONFIG) };
    nrf24l01_reg_en_aa_t reg_en_aa   = { rf24_read_reg8(rf24, NRF24L01_REG_EN_AA) };
    nrf24l01_reg_en_rxaddr_t reg_en_rxaddr = { rf24_read_reg8(rf24, NRF24L01_REG_EN_RXADDR) };
    nrf24l01_reg_setup_aw_t reg_setup_aw = { rf24_read_reg8(rf24, NRF24L01_REG_SETUP_AW) };
    nrf24l01_reg_setup_retr_t reg_setup_retr = { rf24_read_reg8(rf24, NRF24L01_REG_SETUP_RETR) };
    nrf24l01_reg_rf_ch_t reg_rf_ch = { rf24_read_reg8(rf24, NRF24L01_REG_RF_CH) };
    nrf24l01_reg_rf_setup_t reg_rf_setup = { rf24_read_reg8(rf24, NRF24L01_REG_RF_SETUP) };
    nrf24l01_reg_status_t reg_status = { rf24_read_reg8(rf24, NRF24L01_REG_STATUS) };
    nrf24l01_reg_observe_tx_t reg_observe_tx = { rf24_read_reg8(rf24, NRF24L01_REG_OBSERVE_TX) };
    nrf24l01_reg_rpd_t reg_rpd = { rf24_read_reg8(rf24, NRF24L01_REG_RPD) };

    nrf24l01_reg_5byte_addr_t reg_rx_addr_p0;
    nrf24l01_reg_5byte_addr_t reg_rx_addr_p1;
    nrf24l01_reg_5byte_addr_t reg_tx_addr;
    rf24_read_register(rf24, NRF24L01_REG_RX_ADDR_P0, reg_rx_addr_p0.value, rf24->addr_width);
    rf24_read_register(rf24, NRF24L01_REG_RX_ADDR_P1, reg_rx_addr_p1.value, rf24->addr_width);
    rf24_read_register(rf24, NRF24L01_REG_TX_ADDR, reg_tx_addr.value, rf24->addr_width);

    nrf24l01_reg_1byte_addr_t reg_rx_addr_p2 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_ADDR_P2) };
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p3 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_ADDR_P3) };
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p4 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_ADDR_P4) };
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p5 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_ADDR_P5) };
    nrf24l01_reg_rx_pw_p0_t reg_rx_pw_p0 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_PW_P0) };
    nrf24l01_reg_rx_pw_p1_t reg_rx_pw_p1 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_PW_P1) };
    nrf24l01_reg_rx_pw_p2_t reg_rx_pw_p2 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_PW_P2) };
    nrf24l01_reg_rx_pw_p3_t reg_rx_pw_p3 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_PW_P3) };
    nrf24l01_reg_rx_pw_p4_t reg_rx_pw_p4 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_PW_P4) };
    nrf24l01_reg_rx_pw_p5_t reg_rx_pw_p5 = { rf24_read_reg8(rf24, NRF24L01_REG_RX_PW_P5) };
    nrf24l01_reg_fifo_status_t reg_fifo_status = { rf24_read_reg8(rf24, NRF24L01_REG_FIFO_STATUS) };
    nrf24l01_reg_dynpd_t reg_dynpd = { rf24_read_reg8(rf24, NRF24L01_REG_DYNPD) };
    nrf24l01_reg_feature_t reg_feature = { rf24_read_reg8(rf24, NRF24L01_REG_FEATURE) };

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

void rf24_print_status(rf24_t* rf24) {
    nrf24l01_reg_status_t reg_status = rf24_get_status(rf24);

    PRINTF("[07] STATUS      = 0x%02X | RX_DR=%d  TX_DS=%d  MAX_RT=%d  RX_P_NO=%d  TX_FULL=%d\r\n", reg_status.value,
           reg_status.rx_dr, reg_status.tx_ds, reg_status.max_rt, reg_status.rx_p_no,
           reg_status.tx_full);
}
#endif
