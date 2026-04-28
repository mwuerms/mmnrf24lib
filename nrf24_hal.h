/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

#ifndef _NRF24_HAL_H_
#define _NRF24_HAL_H_

// - include -------------------------------------------------------------------
#include "nrf24.h"
#include "main.h"

// - public gpios --------------------------------------------------------------
// CE: nRF24_CE_GPIO_Port, nRF24_CE_Pin
#define nrf24_hal_ce_out()	// already set
#define nrf24_hal_ce_set()		LL_GPIO_SetOutputPin(nRF_CE_GPIO_Port, nRF_CE_Pin)
#define nrf24_hal_ce_clr()		LL_GPIO_ResetOutputPin(nRF_CE_GPIO_Port, nRF_CE_Pin)

// CS: nRF24_CS_GPIO_Port, nRF24_CS_Pin
#define nrf24_hal_cs_output()	// already set
#define nrf24_hal_cs_set()		LL_GPIO_SetOutputPin(nRF_CS_GPIO_Port, nRF_CS_Pin)
#define nrf24_hal_cs_clr()		LL_GPIO_ResetOutputPin(nRF_CS_GPIO_Port, nRF_CS_Pin)

// IRQ: nRF_IRQ_GPIO_Port, nRF_IRQ_Pin, EXTI2_IRQn
#define nrf24_hal_irq_in()				// already set
#define nrf24_hal_irq_get() 			LL_GPIO_IsInputPinSet(nRF_IRQ_GPIO_Port, nRF_IRQ_Pin)
#define nrf24_hal_irq_ie_dis() 			HAL_NVIC_DisableIRQ(nRF_IRQ_EXTI)
#define nrf24_hal_irq_edge_falling()	LL_EXTI_EnableFallingTrig_0_31(nRF_IRQ_EXTI)
#define nrf24_hal_irq_ifg_set() 		LL_EXTI_GenerateSWI_0_31(nRF_IRQ_EXTI)
#define nrf24_hal_irq_ifg_clr() 		LL_EXTI_ClearFlag_0_31(nRF_IRQ_EXTI)

// - public functions ----------------------------------------------------------
void nrf24_hal_Init(void);
void nrf24_hal_Open(void);
void nrf24_hal_Close(void);

uint8_t  nrf24_hal_spi_Transfer(uint8_t b);
uint8_t  nrf24_hal_spi_Transfer_U8_Ctrl_CS(uint8_t b);
uint16_t nrf24_hal_spi_Transfer_Buffer_Blocking(uint8_t *buffer, uint16_t buf_len);
uint16_t nrf24_hal_spi_Transfer_Buffer_Blocking_Ctrl_CS(uint8_t *buffer, uint16_t buf_len);

void nrf24_hal_irq_ie_en(void);

#endif // _NRF24_HAL_H_

