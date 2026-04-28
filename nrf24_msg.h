/**
 * Martin Egli
 * 2026-03-01
 * 
 * nRF24L01 module for STM32
 * based on msprf24, https://github.com/spirilis/msprf24
 */

#ifndef _NRF24_MSG_H_
#define _NRF24_MSG_H_

// - include -------------------------------------------------------------------
#include "main.h"

// - public definitions --------------------------------------------------------
#define nRF24_MSG_SIZE (32)
#define nRF24_MSG_DATA_SIZE (nRF24_MSG_SIZE-2)

typedef struct {
	int16_t acc_x, acc_y, acc_z;
	int16_t temp;
	int16_t gyro_x, gyro_y, gyro_z;
} mpu9250_values_t;

typedef struct {
	float angle;
	int16_t acc_x, acc_y;
	int16_t gyro_z;
} angle_values_t;

typedef struct  __attribute__((packed)) {
	uint8_t id;
	uint8_t len;
	union {
		uint8_t u8[nRF24_MSG_SIZE];
		mpu9250_values_t mpu9250_values;
		angle_values_t angle_values;
	} data;
} nrf24_msg_t;

#define nRF24_MSG_ID_MPU9250_VALUES (0xA1)
#define nRF24_MSG_ID_ANGLE_VALUES (0xA2)
//#define nRF24_MSG_ID_GYRO_VALUES (0xA2)
//#define nRF24_MSG_ID_xx_VALUES (0xA)

#define nRF24_MSG_ID_STRING (0xD0)


// - public variables ----------------------------------------------------------

// - public functions ----------------------------------------------------------

void nrf24_msg_init(void);
void nrf24_msg_send_string(char *str);
void nrf24_msg_send_mpu9250_values(int16_t acc_x, int16_t acc_y, int16_t acc_z, int16_t temp, int16_t gyro_x, int16_t gyro_y, int16_t gyro_z);
void nrf24_msg_send_angle_values(float angle, int16_t acc_x, int16_t acc_y, int16_t gyro_z);

void nrf24_receive_packet(nrf24_msg_t *m);

#endif // _NRF24_MSG_H_
