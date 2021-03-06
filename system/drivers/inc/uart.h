/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 1, 2016
 *	Head file for uart driver.
 **/
#ifndef _UART_H_
#define _UART_H_
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "device.h"
#include "chip.h"

#define ONE_STOP_BIT   1
#define TWO_STOP_BIT   2

#define WORD_LENGTH_5BITS   1
#define WORD_LENGTH_6BITS   2
#define WORD_LENGTH_7BITS   3
#define WORD_LENGTH_8BITS   4

#define PARITY_DISABLE      0
#define PARITY_ODD          1
#define PARITY_EVEN         2
#define PARITY_FORCE_ONE    3
#define PARITY_FORCE_ZERO   4

struct uart_device {
	struct device dev;
	uint32_t baudrate;
	uint8_t word_length;
	uint8_t stop_bits;
	uint8_t parity;
	uint8_t pins;
	const PINMUX_GRP_T *pinctrls;
	SemaphoreHandle_t send_mutex;
	SemaphoreHandle_t recv_mutex;
	QueueHandle_t tx_queue;
	QueueHandle_t rx_queue;
	LPC_USART_T *reg_base;
	IRQn_Type irq;
	uint32_t irq_prior;
	bool initialized;
	bool opened;
};


int uart_device_register(struct uart_device *dev);
void uart_init_all(void);

#endif /* ifndef _UART_H_ */
