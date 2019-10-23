#if !defined(__RF24_H__)
#define __RF24_H__

#include <stdbool.h>

#include "nrf24l01_registers.h"

#include "gpio.h"
#include "spi.h"

#define _BS(bit) (1<<(bit))

typedef enum {
    RF24_1MBPS = 0,
    RF24_2MBPS,
    RF24_250KBPS,
} rf24_datarate_t;

typedef enum {
    RF24_18_dBm = 0,
    RF24_12_dBm,
    RF24_6_dBm,
    RF24_0_dBm,
} rf24_output_power_t;

typedef struct __attribute__((packed)) rf24 {
    GPIO_TypeDef* csn_port;
    uint16_t csn_pin;

    GPIO_TypeDef* ce_port;
    uint16_t ce_pin;

    GPIO_TypeDef* irq_port;
    uint16_t irq_pin;

    SPI_HandleTypeDef* hspi;
    uint16_t spi_timeout;

    uint8_t payload_size;
    uint8_t addr_width;
    rf24_datarate_t datarate;
    uint8_t channel;

    uint8_t pipe0_reading_address[5]; /**< Last address set on pipe 0 for reading. */
} rf24_t;

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
uint32_t txDelay;


rf24_t rf24_get_default_config(void);

bool rf24_init(rf24_t* rf24);

void rf24_power_up(rf24_t* rf24);

void rf24_power_down(rf24_t* rf24);

void rf24_enable(rf24_t* rf24);

void rf24_disable(rf24_t* rf24);

void rf24_set_channel(rf24_t* rf24, uint8_t ch);

uint8_t rf24_get_channel(rf24_t* rf24);

void rf24_set_retries(rf24_t* rf24, uint8_t delay, uint8_t count);

bool rf24_set_datarate(rf24_t* rf24, rf24_datarate_t datarate);

bool rf24_set_output_power(rf24_t* rf24, rf24_output_power_t output_power);

void rf24_flush_rx(rf24_t* rf24);

void rf24_flush_tx(rf24_t* rf24);

void rf24_open_writing_pipe(rf24_t* p_rf24, uint8_t* address);

void rf24_open_reading_pipe(rf24_t* p_rf24, uint8_t pipe_number, uint8_t* address);

void rf24_close_reading_pipe(rf24_t* p_rf24, uint8_t pipe_number);

void rf24_start_listening(rf24_t* p_rf24);

void rf24_stop_listening(rf24_t* p_rf24);


// Read/Write FIFO functions

nrf24l01_reg_status_t rf24_get_status(rf24_t* p_rf24);

bool rf24_available(rf24_t* p_rf24, uint8_t* pipe_number);  // Se não quiser ler o pipe, só mandar NULL

bool rf24_read(rf24_t* p_rf24, uint8_t* buff, uint8_t len);

bool rf24_write(rf24_t* p_rf24, uint8_t* buff, uint8_t len, bool enable_auto_ack);


void rf24_read_all_registers(rf24_t* rf24); // not uplemented (do we need?)


// private?
void rf24_begin_transaction(rf24_t* rf24);

void rf24_end_transaction(rf24_t* rf24);


// read/write register functions (private?)
nrf24l01_reg_status_t rf24_send_command(rf24_t* rf24, nrf24l01_spi_commands_t command);

nrf24l01_reg_status_t rf24_read_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buff, uint8_t len);

uint8_t rf24_read_reg8(rf24_t* rf24, nrf24l01_registers_t reg);

nrf24l01_reg_status_t rf24_write_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buff, uint8_t len);

void rf24_write_reg8(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t value);

nrf24l01_reg_status_t rf24_read_payload(rf24_t* p_rf24, uint8_t* buff, uint8_t len);

nrf24l01_reg_status_t rf24_write_payload(rf24_t* p_rf24, uint8_t* buff, uint8_t len, bool enable_auto_ack);


#ifdef DEBUG
void rf24_dump_registers(rf24_t* rf24);

void rf24_print_status(rf24_t* rf24);
#endif

#endif // __RF24_H__
