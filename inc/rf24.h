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

typedef enum rf24_output_power {
    RF24_18_dBm = 0,
    RF24_12_dBm,
    RF24_6_dBm,
    RF24_0_dBm,
} rf24_output_power_t;

typedef enum rf24_datarate {
    RF24_1MBPS = 0,
    RF24_2MBPS,
    RF24_250KBPS,
} rf24_datarate_t;

typedef struct rf24_dev {
    rf24_platform_t     platform_setup;

    uint8_t             payload_size;
    uint8_t             addr_width;
    rf24_datarate_t     datarate;
    uint8_t             channel;

    uint8_t             pipe0_reading_address[5]; /**< Last address set on pipe 0 for reading. */
} rf24_dev_t;

/*****************************************
 * Public Functions Prototypes
 *****************************************/

rf24_dev_t rf24_get_default_config(void);

rf24_status_t rf24_init(rf24_dev_t* p_dev);

rf24_status_t rf24_power_up(rf24_dev_t* p_dev);

rf24_status_t rf24_power_down(rf24_dev_t* p_dev);

rf24_status_t rf24_set_channel(rf24_dev_t* p_dev, uint8_t ch);

uint8_t rf24_get_channel(rf24_dev_t* p_dev);

rf24_status_t rf24_set_retries(rf24_dev_t* p_dev, uint8_t delay, uint8_t count);

rf24_status_t rf24_set_datarate(rf24_dev_t* p_dev, rf24_datarate_t datarate);

rf24_status_t rf24_set_output_power(rf24_dev_t* p_dev, rf24_output_power_t output_power);

rf24_status_t rf24_flush_rx(rf24_dev_t* p_dev);

rf24_status_t rf24_flush_tx(rf24_dev_t* p_dev);

rf24_status_t rf24_open_writing_pipe(rf24_dev_t* p_dev, uint8_t* address);

rf24_status_t rf24_open_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number, uint8_t* address);

rf24_status_t rf24_close_reading_pipe(rf24_dev_t* p_dev, uint8_t pipe_number);

rf24_status_t rf24_start_listening(rf24_dev_t* p_dev);

rf24_status_t rf24_stop_listening(rf24_dev_t* p_dev);

// Read/Write FIFO functions

rf24_status_t rf24_available(rf24_dev_t* p_dev, uint8_t* pipe_number); // Se não quiser ler o pipe, só mandar NULL

rf24_status_t rf24_read(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len);

rf24_status_t rf24_write(rf24_dev_t* p_dev, uint8_t* buff, uint8_t len, bool enable_auto_ack);


nrf24l01_reg_status_t rf24_get_status(rf24_dev_t* p_dev);


#ifdef DEBUG
void rf24_dump_registers(rf24_dev_t* p_dev);

void rf24_print_status(rf24_dev_t* p_dev);

#endif

#endif // __RF24_H__
