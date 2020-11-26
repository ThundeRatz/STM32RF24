/**
 * @file rf24.h
 *
 * @brief nRF24L01 device related.
 *
 * @author Lucas Haug <lucas.haug@thunderatz.org>
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Daniel Nery <daniel.nery@thunderatz.org>
 *
 * @date 01/2020
 */

#ifndef __RF24_H__
#define __RF24_H__

#include <stdbool.h>
#include <stdio.h>

#include "nrf24l01_registers.h"
#include "rf24_platform.h"

/*****************************************
 * Public Constants
 *****************************************/

#define RF24_ADDRESS_MAX_SIZE 5

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
    RF24_UNKNOWN_ERROR = 8,
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
 * @brief Interruption request type.
 */
typedef struct rf24_irq {
    uint8_t tx_data_sent : 1;
    uint8_t rx_data_ready : 1;
    uint8_t max_retransmits : 1;
} rf24_irq_t;

/**
 * @brief rf24 device type.
 */
typedef struct rf24_dev {
    rf24_platform_t platform_setup;

    uint8_t         payload_size;
    uint8_t         addr_width;
    rf24_datarate_t datarate;
    uint8_t         channel;

    uint8_t         pipe0_reading_address[RF24_ADDRESS_MAX_SIZE];     /**< Last address set on pipe 0 for reading. */
} rf24_dev_t;

/*****************************************
 * Public Functions Prototypes
 *****************************************/

/**
 * @brief Applys default device configuration.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_get_default_config(rf24_dev_t* p_dev);

/**
 * @brief Initilizes device.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_init(rf24_dev_t* p_dev);

/**
 * @brief Power up device.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_power_up(rf24_dev_t* p_dev);

/**
 * @brief Power down device.
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
 * @retval 0xFF Returns 0xFF when got an error, because
 *         channel value can't be bigger than 125.
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
 * @brief Set device data rate.
 *
 * @param p_dev    Pointer to rf24 device.
 * @param datarate Selected data rate.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_set_datarate(rf24_dev_t* p_dev, rf24_datarate_t datarate);

/**
 * @brief Set device output power.
 *
 * @param p_dev         Pointer to rf24 device.
 * @param output_power  Selected output power.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_set_output_power(rf24_dev_t* p_dev, rf24_output_power_t output_power);

/**
 * @brief Flushes receiver FIFO.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_flush_rx(rf24_dev_t* p_dev);

/**
 * @brief Flushes transmitter FIFO.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_flush_tx(rf24_dev_t* p_dev);

/**
 * @brief Enables the device transmission pipe.
 *
 * @param p_dev     Pointer to rf24 device.
 * @param address   Transmitter pipe address.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_open_writing_pipe(rf24_dev_t* p_dev, uint8_t* address);

/**
 * @brief Enables a receiver pipe.
 *
 * @param p_dev         Pointer to rf24 device.
 * @param pipe_number   Number of the pipe to be enabled.
 * @param address       Receiver pipe address.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_open_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number, uint8_t* address);

/**
 * @brief Disables a receiver pipe.
 *
 * @param p_dev       Pointer to rf24 device.
 * @param pipe_number Number of the pipe to be disabled
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_close_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number);

/**
 * @brief Configure the radio as primary receiver and enables it.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_start_listening(rf24_dev_t* p_dev);

/**
 * @brief Disables radio as primary receiver.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_stop_listening(rf24_dev_t* p_dev);

/**
 * @brief Checks if a new payload has arrived.
 *
 * @param p_dev         Pointer to rf24 device.
 * @param pipe_number   Pipe where the available data is.
 *
 * @note To don't ready a pipe, pass NULL as pipe_number argument.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_available(rf24_dev_t* p_dev, uint8_t* pipe_number);

/**
 * @brief Reads the payload avaible in the receiver FIFO.
 *
 * @note Interruption flags related to the receiver are cleared.
 *
 * @param p_dev Pointer to rf24 device.
 * @param buff Pointer to a buffer where the data should be written
 * @param len Maximum number of bytes to read into the buffer
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_read(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len);

/**
 * @brief Writes data in the transmission FIFO, data to be sent to the receiver.
 *
 * @note Be sure to call @ref rf24_open_writing_pipe first to set the
 *       destination of where to write to.
 *
 * @note Interruption flags related to the transmitter are cleared.
 *
 * @param p_dev Pointer to rf24 device.
 * @param buff Pointer to the data to be sent
 * @param len Number of bytes to be sent
 * @param enable_auto_ack Whether auto acknowledgement is enabled or not.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_write(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len, bool enable_auto_ack);

/**
 * @brief Writes data in the transmission FIFO, data to be sent continuously to the receiver.
 *
 * @note Be sure to call @ref rf24_open_writing_pipe first to set the
 *       destination of where to write to.
 *
 * @note Auto acknowledgement and auto retransmit will be disabled in this mode, make sure to
 *       re-configure the device after using this function.
 *
 * @note Interruption flags related to the transmitter are NOT cleared.
 *
 * @param p_dev Pointer to rf24 device.
 * @param buff Pointer to the data to be sent
 * @param len Number of bytes to be sent
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_write_continuously(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len);

/**
 * @brief Gets status register value.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return Status register value.
 * @retval 0xFF Returns 0xFF when got an error, because status
 *         bit 7 only allows 0, so 0xFF is an invalid value.
 */
nrf24l01_reg_status_t rf24_get_status(rf24_dev_t* p_dev);

/**
 * @brief Configurates wich interruptions will active the IRQ pin.
 *
 * @param p_dev Pointer to rf24 device.
 * @param irq_config variable containing wich interruptions will active the IRQ pin.
 *                   Write 1 to the respective struct member to active the interrupt.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_set_irq_configuration(rf24_dev_t* p_dev, rf24_irq_t irq_config);

/**
 * @brief Gets wich type of interruption activated the IRQ pin and resets it.
 *
 * @param p_dev Pointer to rf24 device.
 *
 * @return @ref rf24_irq_t. Each member of the struct indicate if
 *         there was that kind of interruption (1) or not (0).
 */
rf24_irq_t rf24_irq_callback(rf24_dev_t* p_dev);

/**
 * @brief Library delay function.
 *
 * @note This function must be implemented by the user.
 *
 * @param ms Delay in milliseconds.
 *
 * @return @ref rf24_status.
 */
rf24_status_t rf24_delay(uint32_t ms);

#endif // __RF24_H__
