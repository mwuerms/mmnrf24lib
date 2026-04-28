/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

#ifndef _NRF24_CMD_H_
#define _NRF24_CMD_H_

// - include -------------------------------------------------------------------
#include "nrf24.h"
#include "nRF24L01.h"

// - public functions ----------------------------------------------------------
void nrf24_cmd_Init(void);

uint8_t  nrf24_cmd_Transmit(uint8_t cmd, uint8_t *content, uint8_t size);
uint8_t  nrf24_cmd_Read_Register(uint8_t reg_addr, uint8_t *content);
#define nrf24_cmd_Read_Config(c)		nrf24_cmd_Read_Register(RF24_CONFIG, c)
#define nrf24_cmd_Read_EN_AA(c)			nrf24_cmd_Read_Register(RF24_EN_AA, c)
#define nrf24_cmd_Read_EN_RXADDR(c)		nrf24_cmd_Read_Register(RF24_EN_RXADDR, c)
#define nrf24_cmd_Read_Setup_AW(c)		nrf24_cmd_Read_Register(RF24_SETUP_AW, c)
#define nrf24_cmd_Read_Setup_Retr(c)	nrf24_cmd_Read_Register(RF24_SETUP_RETR, c)
#define nrf24_cmd_Read_RF_CH(c)			nrf24_cmd_Read_Register(RF24_RF_CH, c)
#define nrf24_cmd_Read_RF_Setup(c)		nrf24_cmd_Read_Register(RF24_RF_SETUP, c)
#define nrf24_cmd_Read_Status(c)		nrf24_cmd_Read_Register(RF24_STATUS, c)
#define nrf24_cmd_Read_Observe_TX(c)	nrf24_cmd_Read_Register(RF24_OBSERVE_TX, c)
#define nrf24_cmd_Read_RPD(c)			nrf24_cmd_Read_Register(RF24_RPD, c)
#define nrf24_cmd_Read_RX_Addr_P0(c)	nrf24_cmd_Transmit(RF24_R_REGISTER|RF24_RX_ADDR_P0, c, RF24_ADDR_SIZE)
#define nrf24_cmd_Read_RX_Addr_P1(c)	nrf24_cmd_Transmit(RF24_R_REGISTER|RF24_RX_ADDR_P1, c, RF24_ADDR_SIZE)
#define nrf24_cmd_Read_RX_Addr_P2(c)	nrf24_cmd_Read_Register(RF24_RX_ADDR_P2, c)
#define nrf24_cmd_Read_RX_Addr_P3(c)	nrf24_cmd_Read_Register(RF24_RX_ADDR_P3, c)
#define nrf24_cmd_Read_RX_Addr_P4(c)	nrf24_cmd_Read_Register(RF24_RX_ADDR_P4, c)
#define nrf24_cmd_Read_RX_Addr_P5(c)	nrf24_cmd_Read_Register(RF24_RX_ADDR_P5, c)
#define nrf24_cmd_Read_TX_Addr(c)		nrf24_cmd_Transmit(RF24_R_REGISTER|RF24_TX_ADDR, c, RF24_ADDR_SIZE)
#define nrf24_cmd_Read_RX_PW_P0(c)		nrf24_cmd_Read_Register(RF24_RX_PW_P0, c)
#define nrf24_cmd_Read_RX_PW_P1(c)		nrf24_cmd_Read_Register(RF24_RX_PW_P1, c)
#define nrf24_cmd_Read_RX_PW_P2(c)		nrf24_cmd_Read_Register(RF24_RX_PW_P2, c)
#define nrf24_cmd_Read_RX_PW_P3(c)		nrf24_cmd_Read_Register(RF24_RX_PW_P3, c)
#define nrf24_cmd_Read_RX_PW_P4(c)		nrf24_cmd_Read_Register(RF24_RX_PW_P4, c)
#define nrf24_cmd_Read_RX_PW_P5(c)		nrf24_cmd_Read_Register(RF24_RX_PW_P5, c)
#define nrf24_cmd_Read_FIFO_STATUS(c)	nrf24_cmd_Read_Register(RF24_FIFO_STATUS, c)
#define nrf24_cmd_Read_DYNPD(c)			nrf24_cmd_Read_Register(RF24_DYNPD, c)
#define nrf24_cmd_Read_Features(c)		nrf24_cmd_Read_Register(RF24_FEATURE, c)

uint8_t  nrf24_cmd_Write_Register(uint8_t reg_addr, uint8_t content);
#define nrf24_cmd_Write_Config(c)		nrf24_cmd_Write_Register(RF24_CONFIG, c)
#define nrf24_cmd_Write_EN_AA(c)		nrf24_cmd_Write_Register(RF24_EN_AA, c)
#define nrf24_cmd_Write_EN_RXADDR(c)	nrf24_cmd_Write_Register(RF24_EN_RXADDR, c)
#define nrf24_cmd_Write_Setup_AW(c)		nrf24_cmd_Write_Register(RF24_SETUP_AW, c)
#define nrf24_cmd_Write_Setup_Retr(c)	nrf24_cmd_Write_Register(RF24_SETUP_RETR, c)
#define nrf24_cmd_Write_RF_CH(c)		nrf24_cmd_Write_Register(RF24_RF_CH, c)
#define nrf24_cmd_Write_RF_Setup(c)		nrf24_cmd_Write_Register(RF24_RF_SETUP, c)
#define nrf24_cmd_Write_Status(c)		nrf24_cmd_Write_Register(RF24_STATUS, c)
#define nrf24_cmd_Write_Observe_TX(c)	nrf24_cmd_Write_Register(RF24_OBSERVE_TX, c)
#define nrf24_cmd_Write_RPD(c)			nrf24_cmd_Write_Register(RF24_RPD, c)
#define nrf24_cmd_Write_RX_Addr_P0(c)	nrf24_cmd_Transmit(RF24_W_REGISTER|RF24_RX_ADDR_P0, c, RF24_ADDR_SIZE)
#define nrf24_cmd_Write_RX_Addr_P1(c)	nrf24_cmd_Transmit(RF24_W_REGISTER|RF24_RX_ADDR_P1, c, RF24_ADDR_SIZE)
#define nrf24_cmd_Write_RX_Addr_P2(c)	nrf24_cmd_Write_Register(RF24_RX_ADDR_P2, c)
#define nrf24_cmd_Write_RX_Addr_P3(c)	nrf24_cmd_Write_Register(RF24_RX_ADDR_P3, c)
#define nrf24_cmd_Write_RX_Addr_P4(c)	nrf24_cmd_Write_Register(RF24_RX_ADDR_P4, c)
#define nrf24_cmd_Write_RX_Addr_P5(c)	nrf24_cmd_Write_Register(RF24_RX_ADDR_P5, c)
#define nrf24_cmd_Write_TX_Addr(c)		nrf24_cmd_Transmit(RF24_W_REGISTER|RF24_TX_ADDR, c, RF24_ADDR_SIZE)
#define nrf24_cmd_Write_RX_PW_P0(c)		nrf24_cmd_Write_Register(RF24_RX_PW_P0, c)
#define nrf24_cmd_Write_RX_PW_P1(c)		nrf24_cmd_Write_Register(RF24_RX_PW_P1, c)
#define nrf24_cmd_Write_RX_PW_P2(c)		nrf24_cmd_Write_Register(RF24_RX_PW_P2, c)
#define nrf24_cmd_Write_RX_PW_P3(c)		nrf24_cmd_Write_Register(RF24_RX_PW_P3, c)
#define nrf24_cmd_Write_RX_PW_P4(c)		nrf24_cmd_Write_Register(RF24_RX_PW_P4, c)
#define nrf24_cmd_Write_RX_PW_P5(c)		nrf24_cmd_Write_Register(RF24_RX_PW_P5, c)
#define nrf24_cmd_Write_FIFO_STATUS(c)	nrf24_cmd_Write_Register(RF24_FIFO_STATUS, c)
#define nrf24_cmd_Write_DYNPD(c)		nrf24_cmd_Write_Register(RF24_DYNPD, c)
#define nrf24_cmd_Write_Features(c)		nrf24_cmd_Write_Register(RF24_FEATURE, c)

#define nRF24_cmd_Clear_Status()		nrf24_cmd_Write_Status(0x70)

uint8_t  nrf24_cmd_Write_TX_Payload(uint8_t *content, uint8_t size);
uint8_t  nrf24_cmd_Read_RX_Payload(uint8_t *content, uint8_t size);
uint8_t  nrf24_cmd_Flush_TX(void);
uint8_t  nrf24_cmd_Flush_RX(void);

uint8_t  nrf24_cmd_Get_Status(void);

#endif // _NRF24_CMD_H_
