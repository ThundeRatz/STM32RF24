/**
 * @file rf24_debug.c
 *
 * @brief nRF24L01 registers debug related.
 *
 * @author Lucas Haug <lucas.haug@thunderatz.org>
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Daniel Nery <daniel.nery@thunderatz.org>
 *
 * @date 11/2019
 */

#include <stdio.h>

#include "rf24_debug.h"

/*****************************************
 * Private Constants
 *****************************************/

#define RF24_DEBUG_DELAY_MS 100

/*****************************************
 * Private Macros
 *****************************************/

#ifdef DEBUG
#define PRINTF printf
#else
#define PRINTF(...)
#endif

/*****************************************
 * Private Functions Prototypes
 *****************************************/

/**
 * @brief Gets register value.
 *
 * @param p_dev Pointer to rf24 device.
 * @param reg   Register to be read.
 *
 * @return Register value.
 */
uint8_t rf24_debug_read_reg8(rf24_dev_t* p_dev, nrf24l01_registers_t reg);

/*****************************************
 * Public Functions Bodies Definitions
 *****************************************/

void rf24_debug_dump_registers(rf24_dev_t* p_dev) {
    PRINTF("================  REGISTER DUMP ================ \r\n");

    /* NRF24L01_REG_CONFIG */

    nrf24l01_reg_config_t reg_config = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_CONFIG)};
    (void) reg_config;
    PRINTF(
       "[00] CONFIG      = 0x%02X | MASK_RX_DR=%d  MASK_TX_DS=%d  MASK_MAX_RT=%d EN_CRC=%d      CRCO=%d        PWR_UP=%d      PRIM_RX=%d\r\n",
       reg_config.value, reg_config.mask_rx_dr, reg_config.mask_tx_ds, reg_config.mask_max_rt,
       reg_config.en_crc, reg_config.crco, reg_config.pwr_up, reg_config.prim_rx
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_EN_AA */

    nrf24l01_reg_en_aa_t reg_en_aa = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_EN_AA)};
    (void) reg_en_aa;
    PRINTF(
       "[01] EN_AA       = 0x%02X | ENAA_P5=%d     ENAA_P4=%d     ENAA_P3=%d     ENAA_P2=%d     ENAA_P1=%d     ENAA_P0=%d\r\n",
       reg_en_aa.value, reg_en_aa.enaa_p5, reg_en_aa.enaa_p4, reg_en_aa.enaa_p3,
       reg_en_aa.enaa_p2, reg_en_aa.enaa_p1, reg_en_aa.enaa_p0
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_EN_RXADDR */

    nrf24l01_reg_en_rxaddr_t reg_en_rxaddr = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_EN_RXADDR)};
    (void) reg_en_rxaddr;
    PRINTF(
       "[02] EN_RXADDR   = 0x%02X | ERX_P5=%d      ERX_P4=%d      ERX_P3=%d      ERX_P2=%d      ERX_P1=%d      ERX_P0=%d\r\n",
       reg_en_rxaddr.value, reg_en_rxaddr.erx_p5, reg_en_rxaddr.erx_p4, reg_en_rxaddr.erx_p3,
       reg_en_rxaddr.erx_p2, reg_en_rxaddr.erx_p1, reg_en_rxaddr.erx_p0
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_SETUP_AW */

    nrf24l01_reg_setup_aw_t reg_setup_aw = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_SETUP_AW)};
    (void) reg_setup_aw;
    PRINTF(
       "[03] SETUP_AW    = 0x%02X | AW=%d (%s)\r\n", reg_setup_aw.value, reg_setup_aw.aw,
       (reg_setup_aw.aw == 0b00) ? "Illegal" : (reg_setup_aw.aw == 0b01) ? "3 bytes" : (reg_setup_aw.aw == 0b10) ? "4 bytes" : (reg_setup_aw.aw == 0b11) ? "5 bytes" : "???????"
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_SETUP_RETR */

    nrf24l01_reg_setup_retr_t reg_setup_retr = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_SETUP_RETR)};
    (void) reg_setup_retr;
    PRINTF(
       "[04] SETUP_RETR  = 0x%02X | ARD=%d (%d us)  ARC=%d (retransmits)\r\n", reg_setup_retr.value,
       reg_setup_retr.ard, 250 * (1 + reg_setup_retr.ard), reg_setup_retr.arc
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_RF_CH */

    nrf24l01_reg_rf_ch_t reg_rf_ch = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RF_CH)};
    (void) reg_rf_ch;
    PRINTF(
       "[05] RF_CH       = 0x%02X | RF_CH=%d (%d MHz)\r\n", reg_rf_ch.value,
       reg_rf_ch.rf_ch, reg_rf_ch.rf_ch + 2400
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_RF_SETUP */

    nrf24l01_reg_rf_setup_t reg_rf_setup = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RF_SETUP)};
    (void) reg_rf_setup;
    PRINTF(
       "[06] RF_SETUP    = 0x%02X | CONT_WAVE=%d   RF_DR=%d%d (%s) PLL_LOCK=%d  RF_PWR=%d (%sdBm)\r\n",
       reg_rf_setup.value, reg_rf_setup.cont_wave, reg_rf_setup.rf_dr_low, reg_rf_setup.rf_dr_high,
       (reg_rf_setup.rf_dr_low == 0 && reg_rf_setup.rf_dr_high == 0) ? "1Mbps" : (reg_rf_setup.rf_dr_low == 0 && reg_rf_setup.rf_dr_high == 1) ? "2Mbps" : (reg_rf_setup.rf_dr_low == 1 && reg_rf_setup.rf_dr_high == 0) ? "250kbps" : "?????",
       reg_rf_setup.pll_lock, reg_rf_setup.rf_pwr,
       (reg_rf_setup.rf_pwr == 0b00) ? "-18" : (reg_rf_setup.rf_pwr == 0b01) ? "-12" : (reg_rf_setup.rf_pwr == 0b10) ? " -6" : (reg_rf_setup.rf_pwr == 0b11) ? "  0" : "???????"
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_STATUS */

    nrf24l01_reg_status_t reg_status = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_STATUS)};
    (void) reg_status;
    PRINTF(
       "[07] STATUS      = 0x%02X | RX_DR=%d  TX_DS=%d  MAX_RT=%d  RX_P_NO=%d  TX_FULL=%d\r\n",
       reg_status.value, reg_status.rx_dr, reg_status.tx_ds,
       reg_status.max_rt, reg_status.rx_p_no, reg_status.tx_full
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_OBSERVE_TX */

    nrf24l01_reg_observe_tx_t reg_observe_tx = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_OBSERVE_TX)};
    (void) reg_observe_tx;
    PRINTF(
       "[08] OBSERVE_TX  = 0x%02X | PLOS_CNT=%d    ARC_CNT=%d\r\n", reg_observe_tx.value,
       reg_observe_tx.plos_cnt, reg_observe_tx.arc_cnt
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_RPD */

    nrf24l01_reg_rpd_t reg_rpd = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RPD)};
    (void) reg_rpd;
    PRINTF("[09] RPD         = 0x%02X | RPD=%d\r\n", reg_rpd.value, reg_rpd.rpd);

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_RX_ADDR_P0 */

    nrf24l01_reg_5byte_addr_t reg_rx_addr_p0;
    rf24_platform_read_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P0, reg_rx_addr_p0.value, p_dev->addr_width);
    PRINTF(
       "[0A] RX_ADDR_P0  = [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]\r\n", reg_rx_addr_p0.value[0], reg_rx_addr_p0.value[1],
       reg_rx_addr_p0.value[2], reg_rx_addr_p0.value[3], reg_rx_addr_p0.value[4]
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_RX_ADDR_P1 */

    nrf24l01_reg_5byte_addr_t reg_rx_addr_p1;
    rf24_platform_read_register(&(p_dev->platform_setup), NRF24L01_REG_RX_ADDR_P1, reg_rx_addr_p1.value, p_dev->addr_width);
    PRINTF(
       "[0B] RX_ADDR_P1  = [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]\r\n", reg_rx_addr_p1.value[0],
       reg_rx_addr_p1.value[1], reg_rx_addr_p1.value[2], reg_rx_addr_p1.value[3], reg_rx_addr_p1.value[4]
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_RX_ADDR_P2 to NRF24L01_REG_RX_ADDR_P5 */

    nrf24l01_reg_1byte_addr_t reg_rx_addr_p2 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_ADDR_P2)};
    (void) reg_rx_addr_p2;
    PRINTF("[0C] RX_ADDR_P2  = 0x%02X\r\n", reg_rx_addr_p2.value);
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p3 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_ADDR_P3)};
    (void) reg_rx_addr_p3;
    PRINTF("[0D] RX_ADDR_P3  = 0x%02X\r\n", reg_rx_addr_p3.value);
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p4 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_ADDR_P4)};
    (void) reg_rx_addr_p4;
    PRINTF("[0E] RX_ADDR_P4  = 0x%02X\r\n", reg_rx_addr_p4.value);
    nrf24l01_reg_1byte_addr_t reg_rx_addr_p5 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_ADDR_P5)};
    (void) reg_rx_addr_p5;
    PRINTF("[0F] RX_ADDR_P5  = 0x%02X\r\n", reg_rx_addr_p5.value);

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_TX_ADDR */

    nrf24l01_reg_5byte_addr_t reg_tx_addr;
    rf24_platform_read_register(&(p_dev->platform_setup), NRF24L01_REG_TX_ADDR, reg_tx_addr.value, p_dev->addr_width);
    PRINTF(
       "[10] TX_ADDR     = [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]\r\n", reg_tx_addr.value[0],
       reg_tx_addr.value[1], reg_tx_addr.value[2], reg_tx_addr.value[3], reg_tx_addr.value[4]
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_RX_PW_Px */

    nrf24l01_reg_rx_pw_p0_t reg_rx_pw_p0 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_PW_P0)};
    (void) reg_rx_pw_p0;
    PRINTF("[11] RX_PW_P0    = 0x%02X\r\n", reg_rx_pw_p0.value);
    nrf24l01_reg_rx_pw_p1_t reg_rx_pw_p1 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_PW_P1)};
    (void) reg_rx_pw_p1;
    PRINTF("[12] RX_PW_P1    = 0x%02X\r\n", reg_rx_pw_p1.value);
    nrf24l01_reg_rx_pw_p2_t reg_rx_pw_p2 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_PW_P2)};
    (void) reg_rx_pw_p2;
    PRINTF("[13] RX_PW_P2    = 0x%02X\r\n", reg_rx_pw_p2.value);
    nrf24l01_reg_rx_pw_p3_t reg_rx_pw_p3 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_PW_P3)};
    (void) reg_rx_pw_p3;
    PRINTF("[14] RX_PW_P3    = 0x%02X\r\n", reg_rx_pw_p3.value);
    nrf24l01_reg_rx_pw_p4_t reg_rx_pw_p4 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_PW_P4)};
    (void) reg_rx_pw_p4;
    PRINTF("[15] RX_PW_P4    = 0x%02X\r\n", reg_rx_pw_p4.value);
    nrf24l01_reg_rx_pw_p5_t reg_rx_pw_p5 = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_RX_PW_P5)};
    (void) reg_rx_pw_p5;
    PRINTF("[16] RX_PW_P5    = 0x%02X\r\n", reg_rx_pw_p5.value);

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_FIFO_STATUS */

    nrf24l01_reg_fifo_status_t reg_fifo_status = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_FIFO_STATUS)};
    (void) reg_fifo_status;
    PRINTF(
       "[17] FIFO_STATUS = 0x%02X | TX_REUSE=%d  TX_FULL=%d  TX_EMPTY=%d  RX_FULL=%d  RX_EMPTY=%d\r\n", reg_fifo_status.value,
       reg_fifo_status.tx_reuse, reg_fifo_status.tx_full, reg_fifo_status.tx_empty, reg_fifo_status.rx_full, reg_fifo_status.rx_empty
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_DYNPD */

    nrf24l01_reg_dynpd_t reg_dynpd = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_DYNPD)};
    (void) reg_dynpd;
    PRINTF(
       "[1C] DYNPD       = 0x%02X | DPL_P5=%d  DPL_P4=%d  DPL_P3=%d  DPL_P2=%d  DPL_P1=%d  DPL_P0=%d\r\n", reg_dynpd.value,
       reg_dynpd.dpl_p5, reg_dynpd.dpl_p4, reg_dynpd.dpl_p3, reg_dynpd.dpl_p2,
       reg_dynpd.dpl_p1, reg_dynpd.dpl_p0
    );

    rf24_delay(RF24_DEBUG_DELAY_MS);  // Delay is needed so RTT can print the registers values.

    /* NRF24L01_REG_FEATURE */

    nrf24l01_reg_feature_t reg_feature = {rf24_debug_read_reg8(p_dev, NRF24L01_REG_FEATURE)};
    (void) reg_feature;
    PRINTF(
       "[1D] FEATURE     = 0x%02X | EN_DPL=%d  EN_ACK_PAY=%d  EN_DYN_ACK=%d\r\n", reg_feature.value,
       reg_feature.en_dpl, reg_feature.en_ack_pay, reg_feature.en_dyn_ack
    );

    PRINTF("\r\n");
}

void rf24_debug_print_status(rf24_dev_t* p_dev) {
    nrf24l01_reg_status_t reg_status = rf24_get_status(p_dev);
    (void) reg_status;

    PRINTF(
       "[07] STATUS      = 0x%02X | RX_DR=%d  TX_DS=%d  MAX_RT=%d  RX_P_NO=%d  TX_FULL=%d\r\n",
       reg_status.value, reg_status.rx_dr, reg_status.tx_ds,
       reg_status.max_rt, reg_status.rx_p_no, reg_status.tx_full
    );
}

/*****************************************
 * Private Functions Bodies Definitions
 *****************************************/

uint8_t rf24_debug_read_reg8(rf24_dev_t* p_dev, nrf24l01_registers_t reg) {
    uint8_t value;
    rf24_platform_read_reg8(&(p_dev->platform_setup), reg, &value);
    return value;
}
