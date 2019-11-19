/**
 * @file rf24_debug.h
 *
 * @brief nRF24L01 registers debug related.
 *
 * @author Lucas Haug <lucas.haug@thunderatz.org>
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Daniel Nery <daniel.nery@thunderatz.org>
 *
 * @date 11/2019
 */

#ifndef __RF24_DEBUG_H__
#define __RF24_DEBUG_H__

#include "nrf24l01_registers.h"
#include "rf24_platform.h"
#include "rf24.h"

/*****************************************
 * Public Functions Prototypes
 *****************************************/

/**
 * @brief Print all registers values.
 *
 * @param p_dev Pointer to rf24 device.
 */
void rf24_debug_dump_registers(rf24_dev_t* p_dev);

/**
 * @brief Print the status register value.
 *
 * @param p_dev Pointer to rf24 device.
 */
void rf24_debug_print_status(rf24_dev_t* p_dev);

#endif // __RF24_DEBUG_H__
