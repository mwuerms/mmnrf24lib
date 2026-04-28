/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

#ifndef _NRF24_H_
#define _NRF24_H_

// - include -------------------------------------------------------------------
#include "main.h"
#include "nrf24_msg.h"

// - public definitions --------------------------------------------------------
#define nRF24_PACKET_SIZE (32)
typedef struct {
	uint8_t data[nRF24_PACKET_SIZE];
	uint8_t len;
} nrf24_packet_t;

// - public variables ----------------------------------------------------------

// - public functions ----------------------------------------------------------

#define nRF24_ROLE_CENTRAL (1) // controller, always receiving, can send data to client after receiving
#define nRF24_ROLE_PERIPHERIAL (2) // client, wants to send some data, will only receive after sending
#define nRF24_ROLE_TESTING (3) // client, wants to send some data, will only receive after sending
void nrf24_init(uint8_t role);
void nrf24_open(void);
void nrf24_close(void);
void nrf24_process_irq(void);
void nrf24_send_message(nrf24_msg_t *m);

#endif // _NRF24_H_
