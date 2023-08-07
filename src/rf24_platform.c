/**
 * @file rf24_platform.c
 *
 * @brief nRF24L01 platform related.
 *
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Lucas Haug <lucas.haug@thunderatz.org>
 *
 * @date 10/2019
 */

#include "rf24_platform.h"

/*****************************************
 * Private Functions Prototypes
 *****************************************/

/**
 * @brief Begins SPI transaction.
 *
 * @param p_setup Pointer to rf24 instance setup.
 *
 * @return @ref rf24_platform_status.
 */
static rf24_platform_status_t rf24_begin_transaction(rf24_platform_t* p_setup);

/**
 * @brief Ends SPI transaction.
 *
 * @param p_setup Pointer to rf24 instance setup.
 *
 * @return @ref rf24_platform_status.
 */
static rf24_platform_status_t rf24_end_transaction(rf24_platform_t* p_setup);

/*****************************************
 * Public Functions Bodies Definitions
 *****************************************/

rf24_platform_status_t rf24_platform_init(rf24_platform_t* p_setup) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    rf24_end_transaction(p_setup);

    return status;
}

rf24_platform_status_t rf24_platform_enable(rf24_platform_t* p_setup) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(p_setup->ce_port, p_setup->ce_pin, GPIO_PIN_SET);

    return status;
}

rf24_platform_status_t rf24_platform_disable(rf24_platform_t* p_setup) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(p_setup->ce_port, p_setup->ce_pin, GPIO_PIN_RESET);

    return status;
}

rf24_platform_status_t rf24_platform_send_command(rf24_platform_t* p_setup, nrf24l01_spi_commands_t command) {
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(p_setup);

    hal_status = HAL_SPI_TransmitReceive(p_setup->hspi, &command, &(status_reg.value), 1, p_setup->spi_timeout);

    rf24_end_transaction(p_setup);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_platform_get_status(rf24_platform_t* p_setup, nrf24l01_reg_status_t* p_status_reg) {
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    nrf24l01_spi_commands_t command = NRF24L01_COMM_NOP;

    rf24_begin_transaction(p_setup);

    hal_status = HAL_SPI_TransmitReceive(p_setup->hspi, &command, &(status_reg.value), 1, p_setup->spi_timeout);

    rf24_end_transaction(p_setup);

    *p_status_reg = status_reg;

    status = (rf24_platform_status_t) hal_status;

    return status;
}

rf24_platform_status_t rf24_platform_read_register(rf24_platform_t* p_setup, nrf24l01_registers_t reg, uint8_t* buff,
                                                   uint8_t len) {
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(p_setup);

    // Transmit command byte
    uint8_t command = NRF24L01_COMM_R_REGISTER | (NRF24L01_COMM_RW_REGISTER_MASK & reg);
    hal_status = HAL_SPI_TransmitReceive(p_setup->hspi, &command, &(status_reg.value), 1, p_setup->spi_timeout);

    if (hal_status == HAL_OK) {
        hal_status = HAL_SPI_Receive(p_setup->hspi, buff, len, p_setup->spi_timeout);
    }

    rf24_end_transaction(p_setup);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_platform_read_reg8(rf24_platform_t* p_setup, nrf24l01_registers_t reg,
                                               uint8_t* p_reg_value) {
    return rf24_platform_read_register(p_setup, reg, p_reg_value, 1);
}

rf24_platform_status_t rf24_platform_write_register(rf24_platform_t* p_setup, nrf24l01_registers_t reg, uint8_t* buff,
                                                    uint8_t len) {
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(p_setup);

    uint8_t command = NRF24L01_COMM_W_REGISTER | (NRF24L01_COMM_RW_REGISTER_MASK & reg);
    hal_status = HAL_SPI_TransmitReceive(p_setup->hspi, &command, &(status_reg.value), 1, p_setup->spi_timeout);

    if (hal_status == HAL_OK) {
        hal_status = HAL_SPI_Transmit(p_setup->hspi, buff, len, p_setup->spi_timeout);
    }

    rf24_end_transaction(p_setup);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_platform_write_reg8(rf24_platform_t* p_setup, nrf24l01_registers_t reg, uint8_t value) {
    return rf24_platform_write_register(p_setup, reg, &value, 1);
}

rf24_platform_status_t rf24_platform_read_payload(rf24_platform_t* p_setup, uint8_t* buff, uint8_t len) {
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(p_setup);

    uint8_t command = NRF24L01_COMM_R_RX_PAYLOAD;
    hal_status = HAL_SPI_TransmitReceive(p_setup->hspi, &command, &(status_reg.value), 1, p_setup->spi_timeout);

    if (hal_status == HAL_OK) {
        hal_status = HAL_SPI_Receive(p_setup->hspi, buff, len, p_setup->spi_timeout);
    }

    rf24_end_transaction(p_setup);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_platform_write_payload(rf24_platform_t* p_setup, uint8_t* buff, uint8_t len,
                                                   bool enable_auto_ack) {
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(p_setup);

    uint8_t command = enable_auto_ack ? (NRF24L01_COMM_W_TX_PAYLOAD) : (NRF24L01_COMM_W_TX_PAYLOAD_NOACK);
    hal_status = HAL_SPI_TransmitReceive(p_setup->hspi, &command, &(status_reg.value), 1, p_setup->spi_timeout);

    if (hal_status == HAL_OK) {
        hal_status = HAL_SPI_Transmit(p_setup->hspi, buff, len, p_setup->spi_timeout);
    }

    rf24_end_transaction(p_setup);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

/*****************************************
 * Private Functions Bodies Definitions
 *****************************************/

rf24_platform_status_t rf24_begin_transaction(rf24_platform_t* p_setup) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(p_setup->csn_port, p_setup->csn_pin, GPIO_PIN_RESET);

    return status;
}

rf24_platform_status_t rf24_end_transaction(rf24_platform_t* p_setup) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(p_setup->csn_port, p_setup->csn_pin, GPIO_PIN_SET);

    return status;
}
