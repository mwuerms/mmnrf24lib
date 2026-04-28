/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

// - include -------------------------------------------------------------------
#include "nrf24.h"
#include "nrf24_cmd.h"
#include "nrf24_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include "usart.h"

// - private variables for freeRTOS tasks --------------------------------------
osThreadId_t nrf24_rxtx_task_handle = NULL;
uint32_t nrf24_rxtx_task_buffer[ 128 ];
StaticTask_t nrf24_rxtx_task_ctrl_block;
const osThreadAttr_t nrf24_rxtx_task_attributes = {
  .name = "nrf24_rxtx_task",
  .cb_mem = &nrf24_rxtx_task_ctrl_block,
  .cb_size = sizeof(nrf24_rxtx_task_ctrl_block),
  .stack_mem = &nrf24_rxtx_task_buffer[0],
  .stack_size = sizeof(nrf24_rxtx_task_buffer),
  .priority = (osPriority_t) osPriorityLow,
};

osEventFlagsId_t nrf24_event_handle = NULL;
StaticEventGroup_t nrf24_event_ctrl_block;
const osEventFlagsAttr_t nrf24_event_attributes = {
  .name = "nrf24_events",
  .cb_mem = &nrf24_event_ctrl_block,
  .cb_size = sizeof(nrf24_event_ctrl_block),
};

osTimerId_t nrf24_timer_handle = NULL;
StaticTimer_t nrf24_timer_ctrl_block;
const osTimerAttr_t nrf24_timer_attributes = {
  .name = "nrf24_timer",
  .cb_mem = &nrf24_timer_ctrl_block,
  .cb_size = sizeof(nrf24_timer_ctrl_block),
};

#define NRF24_MSG_OUT_QUEUE_SIZE (8)
osMessageQueueId_t nrf24_msg_out_queue_handle = NULL;
uint8_t nrf24_msg_out_queue_buffer[ NRF24_MSG_OUT_QUEUE_SIZE * sizeof(nrf24_msg_t)];
StaticQueue_t nrf24_msg_out_queue_ctrl_block;
const osMessageQueueAttr_t nrf24_msg_out_queue_attributes = {
  .name = "nrf24_msg_out_queue",
  .cb_mem = &nrf24_msg_out_queue_ctrl_block,
  .cb_size = sizeof(nrf24_msg_out_queue_ctrl_block),
  .mq_mem = &nrf24_msg_out_queue_buffer,
  .mq_size = sizeof(nrf24_msg_out_queue_buffer)
};

static void nrf24_rxtx_task_cb(void *argument);
static void nrf24_timer_cb(void *argument);
// - private app functions for freeRTOS ----------------------------------------
static uint16_t nrf24_timer_event = 0;

static inline void nrf24_wait_event(uint32_t ms, uint16_t event) {
	if(ms) {
		nrf24_timer_event = event;
		osTimerStart(nrf24_timer_handle, pdMS_TO_TICKS(ms));
	}
}

#define nRF24_EV_DO_START			BITV(0)
#define nRF24_EV_GUARD_TIMEOUT		BITV(1)
#define nRF24_EV_RX_DONE			BITV(2)
#define nRF24_EV_TX_DONE			BITV(3)
#define nRF24_EV_TX_MAX_RETRY		BITV(4)
#define nRF24_EV_MSG                BITV(5)
#define nRF24_EV_PROCESS_MASK (nRF24_EV_DO_START | nRF24_EV_GUARD_TIMEOUT | nRF24_EV_RX_DONE | nRF24_EV_TX_DONE | nRF24_EV_TX_MAX_RETRY | nRF24_EV_MSG)
static inline void nrf24_send_event(uint16_t ev) {
	osEventFlagsSet(nrf24_event_handle, ev);
}

static inline void nrf24_clear_all_events(void) {
	uint16_t ev;
	// read all events from queue until it is empty
	//while(osMessageQueueGet(nrf24_event_queue_handle, &ev, 0, osWaitForever) == osOK);
	osEventFlagsClear(nrf24_event_handle, 0xFFFFFFFF);
}

// - private variables ---------------------------------------------------------
static struct {
	uint8_t role; // nRF24_ROLE...
	uint8_t state; // nRF24_STATE...
	uint16_t status;// see nRF24_STATUS_ACTIVE ...
	struct {
		uint8_t rxtx_addr[RF24_ADDR_SIZE];
		uint8_t	rf_ch;
		uint8_t pipe;
		uint8_t retries;
	} setup;
	uint8_t cnt;
	nrf24_packet_t in_packet;
	nrf24_packet_t out_packet;
} nrf24_ctrl;

#define nRF24_STATE_CLOSED (0)
#define nRF24_STATE_DO_RX (1)
#define nRF24_STATE_STANDBY (2)
#define nRF24_STATE_TESTING (255)

#define nRF24_STATUS_OFF                0x00
#define nRF24_STATUS_ACTIVE             0x01
#define nRF24_STATUS_OPEN               0x02
#define nRF24_STATUS_RECEIVER_DETECTED  0x04

// - public functions ----------------------------------------------------------

void nrf24_init(uint8_t role) {
	nrf24_ctrl.state = nRF24_STATE_CLOSED;

	nrf24_ctrl.setup.rxtx_addr[0] = 0x00;
	if(role == nRF24_ROLE_CENTRAL) {
		nrf24_ctrl.role = nRF24_ROLE_CENTRAL;
		nrf24_ctrl.setup.rxtx_addr[1] = 0xCE;
	}
	else if (role == nRF24_ROLE_PERIPHERIAL){
		nrf24_ctrl.role = nRF24_ROLE_PERIPHERIAL;
		nrf24_ctrl.setup.rxtx_addr[1] = 0xCE;//0xD1;
	}
	else { // role == nRF24_ROLE_TESTING
		nrf24_ctrl.role = nRF24_ROLE_TESTING;
		nrf24_ctrl.state = nRF24_STATE_TESTING;
	}
	nrf24_ctrl.setup.rxtx_addr[2] = 0xAB;
	nrf24_ctrl.setup.rxtx_addr[3] = 0xAB;
	nrf24_ctrl.setup.rxtx_addr[4] = 0xBA;
	nrf24_ctrl.setup.pipe = 0;
	nrf24_ctrl.setup.rf_ch = 76;
	nrf24_ctrl.setup.retries = 0x25;
	nrf24_ctrl.cnt = '0';

	nrf24_cmd_Init();
	nrf24_hal_Init();
	nrf24_msg_init();

	if(nrf24_timer_handle  == NULL) {
		nrf24_timer_handle = osTimerNew(nrf24_timer_cb, osTimerOnce, NULL, &nrf24_timer_attributes);
	}
	if(nrf24_rxtx_task_handle  == NULL) {
		nrf24_rxtx_task_handle = osThreadNew(nrf24_rxtx_task_cb, NULL, &nrf24_rxtx_task_attributes);
	}
	if(nrf24_event_handle == NULL) {
		nrf24_event_handle = osEventFlagsNew(&nrf24_event_attributes);
	}
	if(nrf24_msg_out_queue_handle == NULL) {
		nrf24_msg_out_queue_handle = osMessageQueueNew(NRF24_MSG_OUT_QUEUE_SIZE, sizeof(nrf24_msg_t), &nrf24_msg_out_queue_attributes);
	}
}

void nrf24_open(void) {
	if(nrf24_ctrl.state == nRF24_STATE_CLOSED) {
		// do open
		nrf24_hal_Open();
		nrf24_wait_event(100, nRF24_EV_DO_START);
	}
	else if(nrf24_ctrl.state == nRF24_STATE_TESTING) {
		nrf24_wait_event(1000, nRF24_EV_TX_DONE);
	}
}

void nrf24_close(void) {
	nrf24_hal_Close();
	nrf24_clear_all_events();
	nrf24_ctrl.state = nRF24_STATE_CLOSED;
}

void nrf24_process_irq(void) {
	uint8_t status;
	nrf24_hal_irq_ie_dis();

	status = nRF24_cmd_Clear_Status();
	if(status & RF24_RX_DR) {
		// data in RX FIFO, maybe ACK + DATA or DATA alone
		nrf24_hal_ce_clr();
		nrf24_send_event(nRF24_EV_RX_DONE);
	}
	if(status & RF24_MAX_RT) {
		nrf24_send_event(nRF24_EV_TX_MAX_RETRY);
	}
}

void nrf24_send_message(nrf24_msg_t *m) {
	osStatus_t ans;
	if((ans = osMessageQueuePut(nrf24_msg_out_queue_handle, m, 0, 0)) == osOK) {
		nrf24_send_event(nRF24_EV_MSG);
	}
	//return ans;
}

// - FreeRTOS callbacks --------------------------------------------------------
static void nrf24_timer_cb(void *argument) {
	nrf24_send_event(nrf24_timer_event);
	/*if(nrf24_timer_event == nRF24_EV_TX_DONE) {
		nrf24_msg_send_string("teststfing!");
	}
	else {
		nrf24_send_event(nrf24_timer_event);
	}*/
}

static void nrf24_rxtx_task_cb(void *argument) {
	uint32_t events = 0;
	uint8_t status, value;
	uint8_t addr_copy[RF24_ADDR_SIZE];
	nrf24_msg_t m;

	while(1) {
		events = osEventFlagsWait(nrf24_event_handle, nRF24_EV_PROCESS_MASK, osFlagsWaitAny, osWaitForever);

		switch(nrf24_ctrl.state) {
			case nRF24_STATE_CLOSED:
				if(events & nRF24_EV_DO_START) {
					status = nrf24_cmd_Write_Config(0x00);
					nRF24_cmd_Clear_Status();
					value = 0x73;
					status = nrf24_cmd_Transmit(0x50, &value, 1);
					status = nrf24_cmd_Write_Features(0x07);
					status = nrf24_cmd_Write_DYNPD(0x3F);
					// setup ADDR, Pipe, rf_channel
					memcpy(addr_copy, nrf24_ctrl.setup.rxtx_addr, RF24_ADDR_SIZE);	// create copy, because nrf24_cmd_Write_TX_Addr() will overwrite addr
					addr_copy[0] += nrf24_ctrl.setup.pipe;
					status = nrf24_cmd_Write_TX_Addr(addr_copy);
					memcpy(addr_copy, nrf24_ctrl.setup.rxtx_addr, RF24_ADDR_SIZE);	// create copy, because nrf24_cmd_Write_RX_Addr_Px() will overwrite addr
					addr_copy[0] += nrf24_ctrl.setup.pipe;
					status = nrf24_cmd_Write_RX_Addr_P0(addr_copy);

					status = nrf24_cmd_Write_EN_AA(0x3F);
					status = nrf24_cmd_Write_EN_RXADDR(0x3F);
					status = nrf24_cmd_Write_RF_CH(nrf24_ctrl.setup.rf_ch);
					status = nrf24_cmd_Write_Setup_Retr(nrf24_ctrl.setup.retries);
					status = nrf24_cmd_Write_RF_Setup(0x07);
					status = nrf24_cmd_Flush_RX();
					status = nrf24_cmd_Flush_TX();

					if(nrf24_ctrl.role == nRF24_ROLE_CENTRAL) {
						nrf24_ctrl.state = nRF24_STATE_DO_RX;
						status = nrf24_cmd_Flush_RX();
						nrf24_hal_irq_ie_en();
						status = nrf24_cmd_Write_Config((RF24_EN_CRC|RF24_CRCO|RF24_PWR_UP|RF24_PRIM_RX));
						nrf24_hal_ce_set();
					}
					else {
						// nRF24_ROLE_PERIPHERIAL
						nrf24_ctrl.state = nRF24_STATE_STANDBY;
						status = nrf24_cmd_Write_Config((RF24_EN_CRC|RF24_CRCO|RF24_PWR_UP));
						nrf24_wait_event(1000, nRF24_EV_TX_DONE);
					}
				}
				break;

			// ---------------------------------------------------------------------
			case nRF24_STATE_DO_RX:
				if(events & nRF24_EV_RX_DONE) {
					// read
					status = nrf24_cmd_Read_FIFO_STATUS(&value);
					status = nrf24_cmd_Read_RX_Payload((uint8_t *)&m, nRF24_PACKET_SIZE);
					status = nrf24_cmd_Flush_RX();
					status = nrf24_cmd_Write_Config((RF24_EN_CRC|RF24_CRCO|RF24_PWR_UP|RF24_PRIM_RX));
					nrf24_receive_packet(&m);
					nrf24_hal_irq_ie_en();
					nrf24_hal_ce_set();
				}
				break;

			// ---------------------------------------------------------------------
			case nRF24_STATE_STANDBY:
				if(events & nRF24_EV_MSG) {
					if(osMessageQueueGet(nrf24_msg_out_queue_handle, &m, 0, 0) == osOK) {
						nrf24_cmd_Flush_TX();
						status = nrf24_cmd_Write_TX_Payload((uint8_t *)&m, m.len+2);
						nrf24_hal_irq_ie_en();
						nrf24_hal_ce_set();
						nrf24_wait_event(2, nRF24_EV_TX_MAX_RETRY);
					}
				}
				if(events & nRF24_EV_TX_MAX_RETRY) {
					nrf24_hal_ce_clr();
					nrf24_wait_event(1000, nRF24_EV_TX_DONE);
				}
				break;

			// ---------------------------------------------------------------------
			case nRF24_STATE_TESTING:
				if(events & nRF24_EV_MSG) {
					if(osMessageQueueGet(nrf24_msg_out_queue_handle, &m, 0, 0) == osOK) {
						nrf24_receive_packet(&m);
					}
					nrf24_wait_event(1000, nRF24_EV_TX_DONE);
				}
				break;

			// ---------------------------------------------------------------------
			default: ;
		}
		osDelay(1);
	}
}
