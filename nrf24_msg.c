/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

// - include -------------------------------------------------------------------
#include "nrf24.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include "usart.h"
#include "../mmlib/str_buf.h"

// - private variables for freeRTOS tasks --------------------------------------

osThreadId_t nrf24_msg_in_task_handle = NULL;
uint32_t nrf24_msg_in_task_buffer[ 128 ];
StaticTask_t nrf24_msg_in_task_ctrl_block;
const osThreadAttr_t nrf24_msg_in_task_attributes = {
  .name = "nrf24_msg_in_task",
  .cb_mem = &nrf24_msg_in_task_ctrl_block,
  .cb_size = sizeof(nrf24_msg_in_task_ctrl_block),
  .stack_mem = &nrf24_msg_in_task_buffer[0],
  .stack_size = sizeof(nrf24_msg_in_task_buffer),
  .priority = (osPriority_t) osPriorityLow,
};

#define NRF24_MSG_IN_QUEUE_SIZE (8)
osMessageQueueId_t nrf24_msg_in_queue_handle = NULL;
uint8_t nrf24_msg_in_queue_buffer[ NRF24_MSG_IN_QUEUE_SIZE * sizeof(nrf24_msg_t)];
StaticQueue_t nrf24_msg_in_queue_ctrl_block;
const osMessageQueueAttr_t nrf24_msg_in_queue_attributes = {
  .name = "nrf24_msg_in_queue",
  .cb_mem = &nrf24_msg_in_queue_ctrl_block,
  .cb_size = sizeof(nrf24_msg_in_queue_ctrl_block),
  .mq_mem = &nrf24_msg_in_queue_buffer,
  .mq_size = sizeof(nrf24_msg_in_queue_buffer)
};

static void nrf24_msg_in_task_cb(void *argument);

#define NRF24_OUT_STR_SIZE (256)
static char nrf24_out_str[NRF24_OUT_STR_SIZE];

void nrf24_msg_init(void) {
	if(nrf24_msg_in_task_handle == NULL) {
		nrf24_msg_in_task_handle = osThreadNew(nrf24_msg_in_task_cb, NULL, &nrf24_msg_in_task_attributes);
	}
	if(nrf24_msg_in_queue_handle == NULL) {
		nrf24_msg_in_queue_handle = osMessageQueueNew(NRF24_MSG_IN_QUEUE_SIZE, sizeof(nrf24_msg_t), &nrf24_msg_in_queue_attributes);
	}
	str_buf_clear(nrf24_out_str, NRF24_OUT_STR_SIZE);
}

void nrf24_msg_send_string(char *str) {
	nrf24_msg_t m;
	uint8_t n;

	// assume str has space in nRF24_MSG_DATA_SIZE for now, check later
	m.id = nRF24_MSG_ID_STRING;
	for(n = 0; n < nRF24_MSG_DATA_SIZE; n++) {
		m.data.u8[n] = str[n];
		if(str[n] == 0) {
			// found end
			break;
		}
	}
	m.len = n;
	nrf24_send_message(&m);
}

void nrf24_msg_send_mpu9250_values(int16_t acc_x, int16_t acc_y, int16_t acc_z, int16_t temp, int16_t gyro_x, int16_t gyro_y, int16_t gyro_z) {
	nrf24_msg_t m;
	m.id = nRF24_MSG_ID_MPU9250_VALUES;
	mpu9250_values_t *values = (mpu9250_values_t *)(&m.data.u8);

	values->acc_x  = acc_x;
	values->acc_y  = acc_y;
	values->acc_z  = acc_z;
	values->temp   = temp;
	values->gyro_x = gyro_x;
	values->gyro_y = gyro_y;
	values->gyro_z = gyro_z;
	m.len = sizeof(mpu9250_values_t);

	nrf24_send_message(&m);
}

void nrf24_msg_send_angle_values(float angle, int16_t acc_x, int16_t acc_y, int16_t gyro_z) {
	nrf24_msg_t m;
	m.id = nRF24_MSG_ID_ANGLE_VALUES;
	angle_values_t *values = (angle_values_t *)(&m.data.u8);

	values->angle = angle;
	values->acc_x  = acc_x;
	values->acc_y  = acc_y;
	values->gyro_z = gyro_z;
	m.len = sizeof(angle_values_t);

	nrf24_send_message(&m);
}

void nrf24_msg_send_remote_ctrl01(int8_t x0, int8_t y0, uint8_t btn0) {
	nrf24_msg_t m;
	m.id = nRF24_MSG_ID_REMOTE_CTRL01;
	remote_ctrl01_t *remote_ctrl01 = (remote_ctrl01_t *)(&m.data.u8);

	remote_ctrl01->joystick[0].x = x0;
	remote_ctrl01->joystick[0].y = y0;
	remote_ctrl01->buttons[0] = btn0;
	m.len = sizeof(remote_ctrl01_t);

	nrf24_send_message(&m);
}



void nrf24_receive_packet(nrf24_msg_t *m) {
	//return
	osMessageQueuePut(nrf24_msg_in_queue_handle, m, 0, 0);
}

static void nrf24_msg_in_task_cb(void *argument) {
	nrf24_msg_t m;
	//mpu9250_values_t *mpu9250_values;
	//angle_values_t *angle_values;
	uint16_t str_len;
	uint8_t new_line;

	while(1) {
		if(osMessageQueueGet(nrf24_msg_in_queue_handle, &m, 0, osWaitForever) == osOK) {
			switch(m.id) {
			// -----------------------------------------------------------------
			case nRF24_MSG_ID_MPU9250_VALUES: {
				//m.len is unimportant
				mpu9250_values_t *mpu9250_values = (mpu9250_values_t *)(&m.data.u8);

				str_len = str_buf_clear(nrf24_out_str, NRF24_OUT_STR_SIZE);
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, mpu9250_values->acc_x);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, mpu9250_values->acc_y);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, mpu9250_values->acc_z);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, mpu9250_values->temp);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, mpu9250_values->gyro_x);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, mpu9250_values->gyro_y);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, mpu9250_values->gyro_z);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, '\n');

				uart_send_buffer((uint8_t *)nrf24_out_str, str_len);
				}
				break;

			// -----------------------------------------------------------------
			case nRF24_MSG_ID_ANGLE_VALUES: {
				//m.len is unimportant
				angle_values_t *angle_values = (angle_values_t *)(&m.data.u8);
				float temp_float = angle_values->angle;

				str_len = str_buf_clear(nrf24_out_str, NRF24_OUT_STR_SIZE);
				str_len = str_buf_append_float(nrf24_out_str, NRF24_OUT_STR_SIZE, temp_float, 2);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, angle_values->acc_x);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, angle_values->acc_y);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int16(nrf24_out_str, NRF24_OUT_STR_SIZE, angle_values->gyro_z);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, '\n');

				uart_send_buffer((uint8_t *)nrf24_out_str, str_len);
				}
				break;

			// -----------------------------------------------------------------
			case nRF24_MSG_ID_REMOTE_CTRL01: {
				//m.len is unimportant
				remote_ctrl01_t *remote_ctrl01 = (remote_ctrl01_t *)(&m.data.u8);

				str_len = str_buf_clear(nrf24_out_str, NRF24_OUT_STR_SIZE);
				str_len = str_buf_append_string(nrf24_out_str, NRF24_OUT_STR_SIZE, "REMOTE_CTRL01: ");
				str_len = str_buf_append_int8(nrf24_out_str, NRF24_OUT_STR_SIZE, remote_ctrl01->joystick[0].x);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_int8(nrf24_out_str, NRF24_OUT_STR_SIZE, remote_ctrl01->joystick[0].y);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, ',');
				str_len = str_buf_append_uint8(nrf24_out_str, NRF24_OUT_STR_SIZE, remote_ctrl01->buttons[0]);
				str_len = str_buf_append_char(nrf24_out_str, NRF24_OUT_STR_SIZE, '\n');

				uart_send_buffer((uint8_t *)nrf24_out_str, str_len);
				}
				break;

			// -----------------------------------------------------------------
			case nRF24_MSG_ID_STRING:
				uart_send_buffer(m.data.u8, m.len);
				new_line = '\n';
				uart_send_buffer(&new_line, 1);
				break;

			// -----------------------------------------------------------------
			default: ;
			}

		}
	}
}
