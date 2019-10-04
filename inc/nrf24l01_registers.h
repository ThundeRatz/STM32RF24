// https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf

#include <stdint.h>

typedef enum nrf24l01_spi_commands {
    NRF24L01_COMM_R_REGISTER = 0x00,
    NRF24L01_COMM_W_REGISTER = 0x20,
    NRF24L01_COMM_RW_REGISTER_MASK = 0x1F,

    NRF24L01_COMM_R_RX_PAYLOAD = 0x61,
    NRF24L01_COMM_W_TX_PAYLOAD = 0xA0,
    NRF24L01_COMM_FLUSH_TX = 0xE1,
    NRF24L01_COMM_FLUSH_RX = 0xE2,
    NRF24L01_COMM_REUSE_TX_PL = 0xE3,
    NRF24L01_COMM_R_RX_PL_WID = 0x60,

    NRF24L01_COMM_W_ACK_PAYLOAD = 0xA8,
    NRF24L01_COMM_W_ACK_PAYLOAD_MASK = 0x07,

    NRF24L01_COMM_W_TX_PAYLOAD_NOACK = 0xB0,
    NRF24L01_COMM_NOP = 0xFF,
} nrf24l01_spi_commands_t;

typedef enum nrf24l01_registers {
    NRF24L01_REG_CONFIG = 0x00,
    NRF24L01_REG_EN_AA = 0x01,
    NRF24L01_REG_EN_RXADDR = 0x02,
    NRF24L01_REG_SETUP_AW = 0x03,
    NRF24L01_REG_SETUP_RETR = 0x04,
    NRF24L01_REG_RF_CH = 0x05,
    NRF24L01_REG_RF_SETUP = 0x06,
    NRF24L01_REG_STATUS = 0x07,
    NRF24L01_REG_OBSERVE_TX = 0x08,
    NRF24L01_REG_RPD = 0x09,
    NRF24L01_REG_RX_ADDR_P0 = 0x0A,
    NRF24L01_REG_RX_ADDR_P1 = 0x0B,
    NRF24L01_REG_RX_ADDR_P2 = 0x0C,
    NRF24L01_REG_RX_ADDR_P3 = 0x0D,
    NRF24L01_REG_RX_ADDR_P4 = 0x0E,
    NRF24L01_REG_RX_ADDR_P5 = 0x0F,
    NRF24L01_REG_TX_ADDR = 0x10,
    NRF24L01_REG_RX_PW_P0 = 0x11,
    NRF24L01_REG_RX_PW_P1 = 0x12,
    NRF24L01_REG_RX_PW_P2 = 0x13,
    NRF24L01_REG_RX_PW_P3 = 0x14,
    NRF24L01_REG_RX_PW_P4 = 0x15,
    NRF24L01_REG_RX_PW_P5 = 0x16,
    NRF24L01_REG_FIFO_STATUS = 0x17,
    NRF24L01_REG_DYNPD = 0x1C,
    NRF24L01_REG_FEATURE = 0x1D,
} nrf24l01_registers_t;

/* Memory Map */
#define NRF_CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define NRF_STATUS 0x07
#define OBSERVE_TX 0x08
#define CD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C
#define FEATURE 0x1D

/* Bit Mnemonics */
//curto cajado melado
#define MASK_RX_DR 6
#define MASK_TX_DS 5
#define MASK_MAX_RT 4
#define EN_CRC 3
#define CRCO 2
#define PWR_UP 1
#define PRIM_RX 0
#define ENAA_P5 5
#define ENAA_P4 4
#define ENAA_P3 3
#define ENAA_P2 2
#define ENAA_P1 1
#define ENAA_P0 0
#define ERX_P5 5
#define ERX_P4 4
#define ERX_P3 3
#define ERX_P2 2
#define ERX_P1 1
#define ERX_P0 0
#define AW 0
#define ARD 4
#define ARC 0
#define PLL_LOCK 4
#define RF_DR 3
#define RF_PWR 6
#define RX_DR 6
#define TX_DS 5
#define MAX_RT 4
#define RX_P_NO 1
#define TX_FULL 0
#define PLOS_CNT 4
#define ARC_CNT 0
#define TX_REUSE 6
#define FIFO_FULL 5
#define TX_EMPTY 4
#define RX_FULL 1
#define RX_EMPTY 0
#define DPL_P5 5
#define DPL_P4 4
#define DPL_P3 3
#define DPL_P2 2
#define DPL_P1 1
#define DPL_P0 0
#define EN_DPL 2
#define EN_ACK_PAY 1
#define EN_DYN_ACK 0

/* Instruction Mnemonics */
#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define REGISTER_MASK 0x1F
#define ACTIVATE 0x50
#define R_RX_PL_WID 0x60
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define W_ACK_PAYLOAD 0xA8
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
#define REUSE_TX_PL 0xE3
#define RF24_NOP 0xFF

/* Non-P omissions */
#define LNA_HCURR 0

/* P model memory Map */
#define RPD 0x09
#define W_TX_PAYLOAD_NO_ACK 0xB0

/* P model bit Mnemonics */
#define RF_DR_LOW 5
#define RF_DR_HIGH 3
#define RF_PWR_LOW 1
#define RF_PWR_HIGH 2

typedef union nrf24l01_reg_config {
    uint8_t value;
    struct {
        uint8_t prim_rx : 1;
        uint8_t pwr_up : 1;
        uint8_t crco : 1;
        uint8_t en_crc : 1;
        uint8_t mask_max_rt : 1;
        uint8_t mask_tx_ds : 1;
        uint8_t mask_rx_dr : 1;
        uint8_t _reserved : 1;
    };
} nrf24l01_reg_config_t;

typedef union nrf24l01_reg_en_aa {
    uint8_t value;
    struct {
        uint8_t enaa_p0 : 1;
        uint8_t enaa_p1 : 1;
        uint8_t enaa_p2 : 1;
        uint8_t enaa_p3 : 1;
        uint8_t enaa_p4 : 1;
        uint8_t enaa_p5 : 1;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_en_aa_t;

typedef union nrf24l01_reg_en_rxaddr {
    uint8_t value;
    struct {
        uint8_t erx_p0 : 1;
        uint8_t erx_p1 : 1;
        uint8_t erx_p2 : 1;
        uint8_t erx_p3 : 1;
        uint8_t erx_p4 : 1;
        uint8_t erx_p5 : 1;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_en_rxaddr_t;

typedef union nrf24l01_reg_setup_aw {
    uint8_t value;
    struct {
        uint8_t aw : 2;
        uint8_t _reserved : 5;
    };
} nrf24l01_reg_setup_aw_t;

typedef union nrf24l01_reg_setup_retr {
    uint8_t value;
    struct {
        uint8_t arc : 4;
        uint8_t ard : 4;
    };
} nrf24l01_reg_setup_retr_t;

typedef union nrf24l01_reg_rf_ch {
    uint8_t value;
    struct {
        uint8_t rf_ch : 7;
        uint8_t _reserved : 1;
    };
} nrf24l01_reg_rf_ch_t;

typedef union nrf24l01_reg_rf_setup {
    uint8_t value;
    struct {
        uint8_t obsolete : 1;
        uint8_t rf_pwr : 2;
        uint8_t rf_dr_high : 1;
        uint8_t pll_lock : 1;
        uint8_t rf_dr_low : 1;
        uint8_t _reserved : 1;
        uint8_t cont_wave : 1;
    };
} nrf24l01_reg_rf_setup_t;

typedef union nrf24l01_reg_status {
    uint8_t value;
    struct {
        uint8_t tx_full : 1;
        uint8_t rx_p_no : 3;
        uint8_t max_rt : 1;
        uint8_t tx_ds : 1;
        uint8_t rx_dr : 1;
        uint8_t _reserved : 1;
    };
} nrf24l01_reg_status_t;

typedef union nrf24l01_reg_observe_tx {
    uint8_t value;
    struct {
        uint8_t arc_cnt : 4;
        uint8_t plos_cnt : 4;
    };
} nrf24l01_reg_observe_tx_t;

typedef union nrf24l01_reg_rpd {
    uint8_t value;
    struct {
        uint8_t rpd : 1;
        uint8_t _reserved : 7;
    };
} nrf24l01_reg_rpd_t;

typedef union nrf24l01_reg_5byte_addr {
    uint8_t value[5];
} nrf24l01_reg_5byte_addr_t;

typedef union nrf24l01_reg_1byte_addr {
    uint8_t value;
} nrf24l01_reg_1byte_addr_t;

typedef union nrf24l01_reg_rx_pw_p0 {
    uint8_t value;
    struct {
        uint8_t rx_pw_p0 : 6;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_rx_pw_p0_t;

typedef union nrf24l01_reg_rx_pw_p1 {
    uint8_t value;
    struct {
        uint8_t rx_pw_p1 : 6;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_rx_pw_p1_t;

typedef union nrf24l01_reg_rx_pw_p2 {
    uint8_t value;
    struct {
        uint8_t rx_pw_p2 : 6;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_rx_pw_p2_t;

typedef union nrf24l01_reg_rx_pw_p3 {
    uint8_t value;
    struct {
        uint8_t rx_pw_p3 : 6;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_rx_pw_p3_t;

typedef union nrf24l01_reg_rx_pw_p4 {
    uint8_t value;
    struct {
        uint8_t rx_pw_p4 : 6;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_rx_pw_p4_t;

typedef union nrf24l01_reg_rx_pw_p5 {
    uint8_t value;
    struct {
        uint8_t rx_pw_p5 : 6;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_rx_pw_p5_t;

typedef union nrf24l01_reg_fifo_status {
    uint8_t value;
    struct {
        uint8_t rx_empty : 1;
        uint8_t rx_full : 1;
        uint8_t _reserved : 2;
        uint8_t tx_empty : 1;
        uint8_t tx_full : 1;
        uint8_t tx_reuse : 1;
        uint8_t __reserved : 1;
    };
} nrf24l01_reg_fifo_status_t;

typedef union nrf24l01_reg_dynpd {
    uint8_t value;
    struct {
        uint8_t dpl_p0 : 1;
        uint8_t dpl_p1 : 1;
        uint8_t dpl_p2 : 1;
        uint8_t dpl_p3 : 1;
        uint8_t dpl_p4 : 1;
        uint8_t dpl_p5 : 1;
        uint8_t _reserved : 2;
    };
} nrf24l01_reg_dynpd_t;

typedef union nrf24l01_reg_feature {
    uint8_t value;
    struct {
        uint8_t en_dyn_ack : 1;
        uint8_t en_ack_pay : 1;
        uint8_t en_dpl : 1;
        uint8_t _reserved : 5;
    };
} nrf24l01_reg_feature_t;
