#if !defined(__RF24_H__)
#define __RF24_H__

#include <stdbool.h>

#include "nrf24l01_registers.h"

#include "gpio.h"
#include "spi.h"

typedef enum {
    RF24_1MBPS = 0,
    RF24_2MBPS,
    RF24_250KBPS,
} rf24_datarate_t;

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
} rf24_t;

rf24_t rf24_get_default_config(void);

bool rf24_init(rf24_t* rf24);

void rf24_enable(rf24_t* rf24);

void rf24_disable(rf24_t* rf24);

void rf24_set_retries(rf24_t* rf24, uint8_t delay, uint8_t count);

bool rf24_set_datarate(rf24_t* rf24, rf24_datarate_t datarate);

void rf24_flush_rx(rf24_t* rf24);

void rf24_flush_tx(rf24_t* rf24);

void rf24_power_up(rf24_t* rf24);

void rf24_read_all_registers(rf24_t* rf24);

#ifdef DEBUG
void rf24_dump_registers(rf24_t* rf24);

void rf24_print_status(rf24_t* rf24);
#endif

// private
void rf24_begin_transaction(rf24_t* rf24);

void rf24_end_transaction(rf24_t* rf24);
//curto cajado melado
nrf24l01_reg_status_t rf24_send_command(rf24_t* rf24, nrf24l01_spi_commands_t command);

nrf24l01_reg_status_t rf24_read_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buf, uint8_t len);

uint8_t rf24_read_reg8(rf24_t* rf24, nrf24l01_registers_t reg);

nrf24l01_reg_status_t rf24_write_register(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t* buf, uint8_t len);

void rf24_write_reg8(rf24_t* rf24, nrf24l01_registers_t reg, uint8_t value);

#endif // __RF24_H__
