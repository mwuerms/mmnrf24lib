/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

// - include -------------------------------------------------------------------
#include "nrf24_cmd.h"
#include "nrf24_hal.h"
#include <string.h>

// - public functions ----------------------------------------------------------
void nrf24_cmd_Init(void) {
	return;
}
uint8_t  nrf24_cmd_Transmit(uint8_t cmd, uint8_t *content, uint8_t size) {
	uint8_t status;
	nrf24_hal_cs_clr();
	status = nrf24_hal_spi_Transfer(cmd);
	nrf24_hal_spi_Transfer_Buffer_Blocking(content, size);
	nrf24_hal_cs_set();
	return status;
}

uint8_t  nrf24_cmd_Read_Register(uint8_t reg_addr, uint8_t *content) {
	uint8_t status;
	nrf24_hal_cs_clr();
	status   = nrf24_hal_spi_Transfer(RF24_R_REGISTER | (reg_addr & RF24_REGISTER_MASK));
	*content = nrf24_hal_spi_Transfer(0);
	nrf24_hal_cs_set();
	return status;
}

uint8_t  nrf24_cmd_Write_Register(uint8_t reg_addr, uint8_t content) {
	uint8_t status;
	nrf24_hal_cs_clr();
	status   = nrf24_hal_spi_Transfer(RF24_W_REGISTER | (reg_addr & RF24_REGISTER_MASK));
	nrf24_hal_spi_Transfer(content);
	nrf24_hal_cs_set();
	return status;
}

uint8_t  nrf24_cmd_Write_TX_Payload(uint8_t *content, uint8_t size) {
	return nrf24_cmd_Transmit(RF24_W_TX_PAYLOAD, content, size);
}

uint8_t  nrf24_cmd_Read_RX_Payload(uint8_t *content, uint8_t size) {
	return nrf24_cmd_Transmit(RF24_R_RX_PAYLOAD, content, size);
}

uint8_t  nrf24_cmd_Flush_TX(void) {
	uint8_t status;
	nrf24_hal_cs_clr();
	status = nrf24_hal_spi_Transfer(RF24_FLUSH_TX);
	nrf24_hal_spi_Transfer(0);
	nrf24_hal_cs_set();
	return status;
}

uint8_t  nrf24_cmd_Flush_RX(void) {
	uint8_t status;
	nrf24_hal_cs_clr();
	status = nrf24_hal_spi_Transfer(RF24_FLUSH_RX);
	nrf24_hal_spi_Transfer(0);
	nrf24_hal_cs_set();
	return status;
}

uint8_t  nrf24_cmd_Get_Status(void) {
	return nrf24_hal_spi_Transfer_U8_Ctrl_CS(RF24_NOP);
}
