/**
 * @file rf24.h
 *
 * @brief nRF24L01 registers related.
 *
 * @author Lucas Haug <lucas.haug@thunderatz.org>
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Daniel Nery <daniel.nery@thunderatz.org>
 *
 * @date 10/2019
 */

#ifndef __RF24_H__
#define __RF24_H__

#include <stdbool.h>
#include <stdio.h>

#include "nrf24l01_registers.h"
#include "rf24_platform.h"

/*****************************************
 * Public Types
 *****************************************/

/**
 * @brief Device status type.
 */
typedef enum rf24_status {
    RF24_SUCCESS = 0,
    RF24_ERROR_CONTROL_INTERFACE = 1,
    RF24_RX_FIFO_EMPTY = 2,
    RF24_TX_FIFO_FULL = 3,
    RF24_MAX_RETRANSMIT = 4,
    RF24_BUFFER_TOO_SMALL = 5,
    RF24_INTERRUPT_NOT_CLEARED = 6,
    RF24_INVALID_PARAMETERS = 7,
    RF24_UNKNOWN_ERRO = 8,
} rf24_status_t;

/**
 * @brief Output power options for transmitter.
 *
 * @note All output power values are negative.
 */
typedef enum rf24_output_power {
    RF24_18_dBm = 0,
    RF24_12_dBm,
    RF24_6_dBm,
    RF24_0_dBm,
} rf24_output_power_t;

/**
 * @brief Datarate options type.
 */
typedef enum rf24_datarate {
    RF24_1MBPS = 0,
    RF24_2MBPS,
    RF24_250KBPS,
} rf24_datarate_t;

/**
 * @brief rf24 device type.
 */
typedef struct rf24_dev {
    rf24_platform_t platform_setup;

    uint8_t         payload_size;
    uint8_t         addr_width;
    rf24_datarate_t datarate;
    uint8_t         channel;

    uint8_t         pipe0_reading_address[5];     /**< Last address set on pipe 0 for reading. */
} rf24_dev_t;

/*****************************************
 * Public Functions Prototypes
 *****************************************/

/**
 * @brief Returns default device configuration.
 *
 * @return Default configuration.
 */
rf24_dev_t rf24_get_default_config(void);

/**
 * @brief Initilizes device.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_init(rf24_dev_t* p_dev);

/**
 * @brief
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_power_up(rf24_dev_t* p_dev);

/**
 * @brief
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_power_down(rf24_dev_t* p_dev);

/**
 * @brief Sets device operating channel.
 *
 * @param p_dev Pointer to rf24 device.
 * @param ch    Channel to be set.
 *
 * @note RF module con operate on frequencies from 2.400GHz to 2.525GHz.
 *       ch parameter must be between 0 and 125, so the operating
 *       frequency will be set by 2.400GHz + ch MHz.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_set_channel(rf24_dev_t* p_dev, uint8_t ch);

/**
 * @brief Gets device operating channel.
 *
 * @note The channel value is between 0 and 125. This value is the
 *       operating frequency width.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return Channel value.
 */
uint8_t rf24_get_channel(rf24_dev_t* p_dev);

/**
 * @brief Sets retries configuration.
 *
 * @param p_dev       Pointer to rf24 device.
 * @param delay_steps Steps of delay
 * @param rt_count    Count of retransmissions.
 *
 * @note Each delay step is 250us. The delay can be calculated by
 *       (steps + 1) * 250 in microseconds. This delay is defined
 *       from end of transmission to start of next transmission.
 *
 * @note Retransmissions count can be up to 15. If set to zero
 *       retransmission is disabled.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_set_retries(rf24_dev_t* p_dev, uint8_t delay_steps, uint8_t rt_count);

/**
 * @brief
 *
 * @param p_dev    Pointer to rf24 device.
 * @param datarate
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_set_datarate(rf24_dev_t* p_dev, rf24_datarate_t datarate);

/**
 * @brief
 *
 * @param p_dev        Pointer to rf24 device.
 * @param output_power
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_set_output_power(rf24_dev_t* p_dev, rf24_output_power_t output_power);

/**
 * @brief
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_flush_rx(rf24_dev_t* p_dev);

/**
 * @brief
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_flush_tx(rf24_dev_t* p_dev);

/**
 * @brief
 *
 * @param p_dev   Pointer to rf24 device.
 * @param address
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_open_writing_pipe(rf24_dev_t* p_dev, uint8_t* address);

/**
 * @brief
 *
 * @param p_dev       Pointer to rf24 device.
 * @param pipe_number
 * @param address
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_open_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number, uint8_t* address);

/**
 * @brief
 *
 * @param p_dev       Pointer to rf24 device.
 * @param pipe_number
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_close_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number);

/**
 * @brief
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_start_listening(rf24_dev_t* p_dev);

/**
 * @brief
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_stop_listening(rf24_dev_t* p_dev);

/**
 * @brief
 *
 * @param p_dev       Pointer to rf24 device.
 * @param pipe_number
 *
 * @note To don't ready a pipe, pass NULL as pipe_number argument.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_available(rf24_dev_t* p_dev, uint8_t* pipe_number);

/**
 * @brief
 *
 * @param p_dev Pointer to rf24 device.
 * @param buff
 * @param len
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_read(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len);

/**
 * @brief
 *
 * @param p_dev           Pointer to rf24 device.
 * @param buff
 * @param len
 * @param enable_auto_ack Whether auto acknowledgement is enabled or not.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_write(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len, bool enable_auto_ack);

/**
 * @brief Gets status register value.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return Status register value.
 * @retval 0xFF Returns 0xFF when got an error, because status bit
 *              7 only allows 0, so 0xFF is an invalid value.
 */
nrf24l01_reg_status_t rf24_get_status(rf24_dev_t* p_dev);

/**
 * @brief Library delay function.
 *
 * @note This function must be implemented by the user.
 *
 * @param ms Delay in miliseconds.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_delay(uint32_t ms);

#endif // __RF24_H__
