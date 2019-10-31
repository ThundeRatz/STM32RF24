/**
 * @file rf24_platform.h
 *
 * @brief nRF24L01 registers related.
 *
 * @author Lucas Haug <lucas.haug@thunderatz.org>
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 *
 * @date 10/2019
 */

#ifndef __RF24_PLATFORM_H__
#define __RF24_PLATFORM_H__

#include <stdbool.h>

#include "gpio.h"
#include "spi.h"

#include "nrf24l01_registers.h"

/*****************************************
 * Public Types
 *****************************************/

/**
 * @brief Platform status type.
 */
typedef enum rf24_platform_status {
    RF24_PLATFORM_SUCCESS = 0x00U,
    RF24_PLATFORM_ERROR = 0x01U,
    RF24_PLATFORM_SPI_BUSY = 0x02U,
    RF24_PLATFORM_SPI_TIMEOUT = 0x03U,
} rf24_platform_status_t;

typedef enum rf24_datarate {  // NAO DEVIA ESTAR AQUI
    RF24_1MBPS = 0,
    RF24_2MBPS,
    RF24_250KBPS,
} rf24_datarate_t;

/**
 * @brief Platform hardware related.
 */
typedef struct __attribute__((packed)) rf24 {
    GPIO_TypeDef*      csn_port;
    uint16_t           csn_pin;

    GPIO_TypeDef*      ce_port;
    uint16_t           ce_pin;

    GPIO_TypeDef*      irq_port;
    uint16_t           irq_pin;

    SPI_HandleTypeDef* hspi;
    uint16_t           spi_timeout;

    uint8_t            payload_size;
    uint8_t            addr_width;
    rf24_datarate_t    datarate;
    uint8_t            channel;

    uint8_t            pipe0_reading_address[5]; /**< Last address set on pipe 0 for reading. */
} rf24_t;

/*****************************************
 * Public Functions Prototypes
 *****************************************/

/**
 * @brief Initializes platform.
 *
 * @param p_rf24 Pointer to rf24 instance
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_platform_init(rf24_t* p_rf24);

/**
 * @brief Enables the rf24 chip.
 *
 * @param p_rf24 Pointer to rf24 instance
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_enable(rf24_t* rf24);

/**
 * @brief Disables the rf24 chip.
 *
 * @param p_rf24 Pointer to rf24 instance
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_disable(rf24_t* rf24);

/**
 * @brief Send SPI command.
 *
 * @param p_rf24    Pointer to rf24 instance
 * @param command   SPI command @ref nrf24l01_spi_commands
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_send_command(rf24_t* rf24, nrf24l01_spi_commands_t command);

/**
 * @brief Gets status register value.
 *
 * @param p_rf24         Pointer to rf24 instance
 * @param p_status_reg   Pointer to a variable to store the status.
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_platform_get_status(rf24_t *rf24, nrf24l01_reg_status_t* p_status_reg);

/**
 * @brief Read register from device.
 *
 * @param p_rf24    Pointer to rf24 instance
 * @param reg       Register to be read
 * @param buff      Buffer to store register value
 * @param len       Buffer lenght
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_read_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buff, uint8_t len);

/**
 * @brief Read a 8 bit register from device.
 *
 * @param p_rf24      Pointer to rf24 instance
 * @param reg         Register to be read
 * @param p_reg_value Pointer to a variable to store the value from the register.
 *
 * @return Register value.
 */
rf24_platform_status_t rf24_read_reg8(rf24_t *rf24, nrf24l01_registers_t reg, uint8_t* p_reg_value );

/**
 * @brief Write device register.
 *
 * @param p_rf24    Pointer to rf24 instance
 * @param reg       Register to be written
 * @param buff      Buffer to store register value
 * @param len       Buffer lenght
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_write_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buff, uint8_t len);

/**
 * @brief Write a 8 bit device register.
 *
 * @param p_rf24    Pointer to rf24 instance
 * @param reg       Register to be written
 * @param value     Value to writein theregister
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_write_reg8(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t value);

/**
 * @brief Read a payload from device Rx FIFO.
 *
 * @param p_rf24    Pointer to rf24 instance
 * @param buff      Buffer to store the payload data
 * @param len       Payload lenght
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_read_payload(rf24_t* p_rf24, uint8_t* buff, uint8_t len);

/**
 * @brief Write payload in device Tx FIFO.
 *
 * @param p_rf24            Pointer to rf24 instance
 * @param buff              Buffer to store the payload data
 * @param len               Payload lenght
 * @param enable_auto_ack   Tells the receiver if a acknowledgement packet is expected
 *
 * @note To use the enable_auto_ack feature, the user must set the EN_DYN_ACK bit
 *       from the FEATURE register. This will allow to control which payload
 *       requires acknowledgement dynamically.
 *
 * @return @ref rf24_platform_status.
 */
rf24_platform_status_t rf24_write_payload(rf24_t* p_rf24, uint8_t* buff, uint8_t len, bool enable_auto_ack);

#endif // __RF24_PLATFORM_H__
