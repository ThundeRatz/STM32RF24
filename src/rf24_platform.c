/**
 * @file rf24_platform.c
 *
 * @brief nRF24L01 registers related.
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
 * @param p_rf24 Pointer to rf24 instance
 *
 * @return @ref rf24_platform_status.
 */
static rf24_platform_status_t rf24_begin_transaction(rf24_t* rf24);

/**
 * @brief Ends SPI transaction.
 *
 * @param p_rf24 Pointer to rf24 instance
 *
 * @return @ref rf24_platform_status.
 */
static rf24_platform_status_t rf24_end_transaction(rf24_t* rf24);

/*****************************************
 * Public Functions Bodies Definitions
 *****************************************/

rf24_platform_status_t rf24_platform_init(rf24_t* p_rf24) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    rf24_end_transaction(p_rf24);

    return status;
}

rf24_platform_status_t rf24_enable(rf24_t *rf24) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(rf24->ce_port, rf24->ce_pin, GPIO_PIN_SET);

    return status;
}

rf24_platform_status_t rf24_disable(rf24_t *rf24) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(rf24->ce_port, rf24->ce_pin, GPIO_PIN_RESET);

    return status;
}

rf24_platform_status_t rf24_send_command(rf24_t *rf24, nrf24l01_spi_commands_t command)
{
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(rf24);

    hal_status = HAL_SPI_TransmitReceive(rf24->hspi, &command, &(status_reg.value), 1, rf24->spi_timeout);

    rf24_end_transaction(rf24);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_platform_get_status(rf24_t *rf24, nrf24l01_reg_status_t* p_status_reg)
{
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    nrf24l01_spi_commands_t command = NRF24L01_COMM_NOP;

    rf24_begin_transaction(rf24);

    hal_status = HAL_SPI_TransmitReceive(rf24->hspi, &command, &(status_reg.value), 1, rf24->spi_timeout);

    rf24_end_transaction(rf24);

    *p_status_reg = status_reg;

    status = (rf24_platform_status_t) hal_status;

    return status;
}

rf24_platform_status_t rf24_read_register(rf24_t *rf24, nrf24l01_registers_t reg, uint8_t *buff, uint8_t len)
{
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(rf24);

    // Transmit command byte
    uint8_t command = NRF24L01_COMM_R_REGISTER | (NRF24L01_COMM_RW_REGISTER_MASK & reg);
    hal_status = HAL_SPI_TransmitReceive(rf24->hspi, &command, &(status_reg.value), 1, rf24->spi_timeout);

    if (hal_status == HAL_OK) {
        HAL_SPI_Receive(rf24->hspi, buff, len, rf24->spi_timeout);
    }

    rf24_end_transaction(rf24);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_read_reg8(rf24_t *rf24, nrf24l01_registers_t reg, uint8_t* p_reg_value)
{
    return rf24_read_register(rf24, reg, p_reg_value, 1);
}

rf24_platform_status_t rf24_write_register(rf24_t *rf24, nrf24l01_registers_t reg, uint8_t *buff, uint8_t len)
{
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(rf24);

    uint8_t command = NRF24L01_COMM_W_REGISTER | (NRF24L01_COMM_RW_REGISTER_MASK & reg);
    hal_status = HAL_SPI_TransmitReceive(rf24->hspi, &command, &(status_reg.value), 1, rf24->spi_timeout);

    if (hal_status == HAL_OK) {
        HAL_SPI_Transmit(rf24->hspi, buff, len, rf24->spi_timeout);
    }

    rf24_end_transaction(rf24);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_write_reg8(rf24_t *rf24, nrf24l01_registers_t reg, uint8_t value)
{
    return rf24_write_register(rf24, reg, &value, 1);
}

rf24_platform_status_t rf24_read_payload(rf24_t *p_rf24, uint8_t *buff, uint8_t len)
{
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(p_rf24);

    uint8_t command = NRF24L01_COMM_R_RX_PAYLOAD;
    hal_status = HAL_SPI_TransmitReceive(p_rf24->hspi, &command, &(status_reg.value), 1, p_rf24->spi_timeout);

    if (hal_status == HAL_OK) {
        HAL_SPI_Receive(p_rf24->hspi, buff, len, p_rf24->spi_timeout);
    }

    rf24_end_transaction(p_rf24);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

rf24_platform_status_t rf24_write_payload(rf24_t *p_rf24, uint8_t *buff, uint8_t len, bool enable_auto_ack)
{
    rf24_platform_status_t status;
    HAL_StatusTypeDef hal_status;
    nrf24l01_reg_status_t status_reg;

    rf24_begin_transaction(p_rf24);

    uint8_t command = enable_auto_ack ? (NRF24L01_COMM_W_TX_PAYLOAD) : (NRF24L01_COMM_W_TX_PAYLOAD_NOACK);
    hal_status = HAL_SPI_TransmitReceive(p_rf24->hspi, &command, &(status_reg.value), 1, p_rf24->spi_timeout);

    if (hal_status == HAL_OK) {
        HAL_SPI_Transmit(p_rf24->hspi, buff, len, p_rf24->spi_timeout);
    }

    rf24_end_transaction(p_rf24);

    status = (rf24_platform_status_t) hal_status;
    return status;
}

/*****************************************
 * Private Functions Bodies Definitions
 *****************************************/

rf24_platform_status_t rf24_begin_transaction(rf24_t *rf24) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(rf24->csn_port, rf24->csn_pin, GPIO_PIN_RESET);

    return status;
}

rf24_platform_status_t rf24_end_transaction(rf24_t *rf24) {
    rf24_platform_status_t status = RF24_PLATFORM_SUCCESS;

    HAL_GPIO_WritePin(rf24->csn_port, rf24->csn_pin, GPIO_PIN_SET);

    return status;
}
