/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

// - include -------------------------------------------------------------------
#include "nrf24_hal.h"
#include "main.h"
#include "spi.h"

// - private defines -----------------------------------------------------------
#define nRF24_SPI SPI1

// - GPIOs ---------------------------------------------------------------------

// - public functions ----------------------------------------------------------
void nrf24_hal_Init(void) {
	// gpio
	nrf24_hal_ce_out();
	nrf24_hal_ce_clr();
	nrf24_hal_cs_output();
	nrf24_hal_cs_clr();
	nrf24_hal_irq_in();
	nrf24_hal_irq_ifg_clr();
	nrf24_hal_irq_ie_dis();
}

void nrf24_hal_Open(void) {
	nrf24_hal_ce_clr();
	nrf24_hal_cs_set();
	nrf24_hal_irq_ifg_clr();
	nrf24_hal_irq_ie_en();
	nrf24_hal_irq_in();
	nrf24_hal_irq_edge_falling();

	// SPI1 konfigurieren
	LL_SPI_SetMode(nRF24_SPI, LL_SPI_MODE_MASTER);
	LL_SPI_SetDataWidth(nRF24_SPI, LL_SPI_DATAWIDTH_8BIT);
	LL_SPI_SetClockPolarity(nRF24_SPI, LL_SPI_POLARITY_LOW);
	LL_SPI_SetClockPhase(nRF24_SPI, LL_SPI_PHASE_1EDGE);
	LL_SPI_Enable(nRF24_SPI);

}

void nrf24_hal_Close(void) {
	nrf24_hal_irq_ie_dis();
	nrf24_hal_irq_ifg_clr();
	nrf24_hal_irq_in();
	nrf24_hal_ce_clr();
	nrf24_hal_cs_clr();
	LL_SPI_Disable(nRF24_SPI);
}

void nrf24_hal_irq_ie_en(void) {
	nrf24_hal_irq_ie_dis();
	NVIC_SetPriority(nRF_IRQ_EXTI_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),8, 0));
	nrf24_hal_irq_edge_falling();
	nrf24_hal_irq_ifg_clr();
	HAL_NVIC_EnableIRQ(nRF_IRQ_EXTI_IRQn);
	if(nrf24_hal_irq_get() == 0) {
		// irq is already low: set ifg
		nrf24_hal_irq_ifg_set();
	}
}

static inline uint8_t spi_transfer(uint8_t b) {
	while (!LL_SPI_IsActiveFlag_TXE(nRF24_SPI));
	LL_SPI_TransmitData8(nRF24_SPI, b);
	while (!LL_SPI_IsActiveFlag_RXNE(nRF24_SPI));
	return LL_SPI_ReceiveData8(nRF24_SPI);
}

uint8_t  nrf24_hal_spi_Transfer(uint8_t byte) {
	return spi_transfer(byte);
}

uint8_t  nrf24_hal_spi_Transfer_U8_Ctrl_CS(uint8_t byte) {
	uint8_t rb;
	nrf24_hal_cs_clr();
	rb = spi_transfer(byte);
	nrf24_hal_cs_set();
	return rb;
}

uint16_t nrf24_hal_spi_Transfer_Buffer_Blocking(uint8_t *buffer, uint16_t buf_len) {
	uint16_t n;
	if(buf_len == 0) {
		// error, nothing to send
		return 0;
	}
	for(n = 0; n < buf_len; n++) {
		buffer[n] = spi_transfer(buffer[n]);
	}
	return n;
}

uint16_t nrf24_hal_spi_Transfer_Buffer_Blocking_Ctrl_CS(uint8_t *buffer, uint16_t buf_len) {
	uint16_t n;
	nrf24_hal_cs_clr();
	n = nrf24_hal_spi_Transfer_Buffer_Blocking(buffer, buf_len);
	nrf24_hal_cs_set();
	return n;
}
