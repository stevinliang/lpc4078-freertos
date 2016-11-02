/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 1, 2016
 *	uart driver.
 **/
#include "uart.h"
#include "board_api.h"
#define UART_PORT_NUM 5

static struct uart_device *uart_devices[UART_PORT_NUM];

/**
 * uart_set_pinctrls - configure pins which uart used.
 * @dev: pointer to a uart_device structure.
 *
 */
static void uart_set_pinctrls(struct uart_device *dev)
{
	Chip_IOCON_SetPinMuxing(LPC_IOCON, dev->pinctrls, dev->pins);
}


static void uart_set_baudrate(struct uart_device *dev, uint32_t baudrate)
{
	dev->baudrate = baudrate;
	Chip_UART_SetBaud(dev->reg_base, baudrate);
}

static void uart_set_line_control(struct uart_device *dev,
		uint8_t word_length, uint8_t stop_bits, uint8_t parity)
{
	uint32_t line_ctl = 0;

	/* Set word length: 5, 6, 7, 8 bits*/
	dev->word_length = word_length;
	switch (word_length) {
	case WORD_LENGTH_5BITS:
		line_ctl |= UART_LCR_WLEN5;
		break;
	case WORD_LENGTH_6BITS:
		line_ctl |= UART_LCR_WLEN6;
		break;
	case WORD_LENGTH_7BITS:
		line_ctl |= UART_LCR_WLEN7;
		break;
	case WORD_LENGTH_8BITS:
		line_ctl |= UART_LCR_WLEN8;
		break;
	default:
		line_ctl |= UART_LCR_WLEN8;
		dev->word_length = WORD_LENGTH_8BITS;
		break;
	}

	/* Set stop bits: 1 or 2 stop bits */
	dev->stop_bits = stop_bits;
	switch (stop_bits) {
	case ONE_STOP_BIT:
		line_ctl |= UART_LCR_SBS_1BIT;
		break;
	case TWO_STOP_BIT:
		line_ctl |= UART_LCR_SBS_2BIT;
		break;
	default:
		line_ctl |= UART_LCR_SBS_1BIT;
		dev->stop_bits = ONE_STOP_BIT;
		break;
	}

	/* Set Parity: disable, odd, even, force 1, force 0, default
	 * disable */
	dev->parity = parity;
	switch (parity) {
	case PARITY_DISABLE:
		line_ctl |= UART_LCR_PARITY_DIS;
		break;
	case PARITY_ODD:
		line_ctl |= UART_LCR_PARITY_EN | UART_LCR_PARITY_ODD;
		break;
	case PARITY_EVEN:
		line_ctl |= UART_LCR_PARITY_EN | UART_LCR_PARITY_EVEN;
		break;
	case PARITY_FORCE_ONE:
		line_ctl |= UART_LCR_PARITY_EN | UART_LCR_PARITY_F_1;
		break;
	case PARITY_FORCE_ZERO:
		line_ctl |= UART_LCR_PARITY_EN | UART_LCR_PARITY_F_0;
		break;
	default:
		line_ctl |= UART_LCR_PARITY_DIS;
		dev->parity = PARITY_DISABLE;
		break;
	}

	Chip_UART_ConfigData(dev->reg_base, line_ctl);
	Chip_UART_TXEnable(dev->reg_base);
}


int32_t uart_send(struct uart_device *dev, uint8_t *buf, int32_t len)
{
	int i = 0;

	if (xSemaphoreTake(dev->send_mutex, portMAX_DELAY) == pdTRUE) {
		for (i = 0; i < len; i++) {
			while ((Chip_UART_ReadLineStatus(dev->reg_base) & UART_LSR_THRE) == 0) {}
			Chip_UART_SendByte(dev->reg_base, buf[i]);
		}
		xSemaphoreGive(dev->send_mutex);
		return len;
	}

	return 0;
}


int32_t uart_recv(struct uart_device *dev, uint8_t *buf, int32_t len)
{
	int i = 0;
	while (1) {
		while (!Chip_UART_ReadLineStatus(dev->reg_base) & UART_LSR_RDR) {}
		buf[i++] = Chip_UART_ReadByte(dev->reg_base);
		if (i == len)
			break;
	}
	return len;
}


static void uart_init(struct uart_device *dev)
{
	if (dev->initialized)
		return;

	dev->send_mutex = xSemaphoreCreateMutex();
	dev->recv_mutex = xSemaphoreCreateMutex();
	if (dev->send_mutex && dev->recv_mutex) {
		uart_set_pinctrls(dev);
		Chip_UART_Init(dev->reg_base);
		uart_set_baudrate(dev, dev->baudrate);
		uart_set_line_control(dev, dev->word_length,
				dev->stop_bits, dev->parity);
		dev->initialized = true;
	}
}
static void uart_deinit(struct uart_device *dev)
{
	if (!dev->initialized)
		return;
	vSemaphoreDelete(dev->send_mutex);
	vSemaphoreDelete(dev->recv_mutex);
	Chip_UART_DeInit(dev->reg_base);
	dev->initialized = false;

}

static int uart_dev_index(struct uart_device *dev)
{
	int index = -1;

	switch ((uint32_t)dev->reg_base) {
	case (uint32_t)LPC_UART0:
		index = 0;
		break;
	case (uint32_t)LPC_UART1:
		index = 1;
		break;
	case (uint32_t)LPC_UART2:
		index = 2;
		break;
	case (uint32_t)LPC_UART3:
		index = 3;
		break;
	case (uint32_t)LPC_UART4:
		index = 4;
		break;
	default:
		break;
	}

	return index;
}

int uart_device_register(struct uart_device *dev)
{
	int index = uart_dev_index(dev);

	if (index < 0)
		return -1;

	if (uart_devices[index])
		return -1;

	uart_devices[index] = dev;

	uart_init(dev);

	return 0;
}

void uart_device_unregister(struct uart_device *dev)
{
	int index = uart_dev_index(dev);

	if (index < 0)
		return;

	if (!uart_devices[index])
		return;
	uart_deinit(dev);
	uart_devices[index] = NULL;

}

static const PINMUX_GRP_T uart0_pinctrls[] = {
	{0, 2, (IOCON_FUNC1 | IOCON_MODE_INACT)},
	{0, 3, (IOCON_FUNC1 | IOCON_MODE_INACT)},
};

struct uart_device uart0 = {
	.baudrate = 115200,
	.word_length = WORD_LENGTH_8BITS,
	.stop_bits = ONE_STOP_BIT,
	.parity = PARITY_DISABLE,
	.send_mutex = NULL,
	.recv_mutex = NULL,
	.reg_base = LPC_UART0,
	.pinctrls = &uart0_pinctrls,
	.pins = sizeof(uart0_pinctrls) / sizeof(uart0_pinctrls[0]),
};


void uart_init_all()
{
	uart_device_register(&uart0);
}
