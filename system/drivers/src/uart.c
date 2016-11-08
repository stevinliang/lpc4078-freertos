/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 1, 2016
 *	uart driver for lpc40xx device.
 *
 * History:
 *	Nov 1, 2016: polling mechanism implementation.
 *	Nov 3, 2016: interrupt mechanism implementation.
 **/

#include "uart.h"
#include "task.h"
#define UART_PORT_NUM 5
#define UART_RECV_BUFFER_LEN  256
#define UART_SEND_BUFFER_LEN  128

static struct uart_device *uart_devices[UART_PORT_NUM];

/**
 * uart_set_pinctrls -- configure pins which uart used.
 * @dev: pointer to a uart_device structure.
 */
static void uart_set_pinctrls(struct uart_device *dev)
{
	Chip_IOCON_SetPinMuxing(LPC_IOCON, dev->pinctrls, dev->pins);
}

/**
 * uart_set_baudrate -- Set baudrate of uart device.
 * @dev: uart device will change its baudrate
 * @baudrate: baudrate will be set
 */
static void uart_set_baudrate(struct uart_device *dev, uint32_t baudrate)
{
	dev->baudrate = baudrate;
	Chip_UART_SetBaud(dev->reg_base, baudrate);
}

/**
 * uart_set_line_control -- set the line control of the uart device
 * @word_length: word length to be set.
 * @stop_bits: one stop bit or two stop bits
 * @parity: parity check.
 */
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
	Chip_UART_SetupFIFOS(dev->reg_base, (UART_FCR_FIFO_EN | UART_FCR_RX_RS |
				UART_FCR_TX_RS | UART_FCR_TRG_LEV3));
	Chip_UART_IntEnable(dev->reg_base, UART_IER_RBRINT | UART_IER_RLSINT);
	NVIC_SetPriority(dev->irq, dev->irq_prior);
	NVIC_EnableIRQ(dev->irq);
	Chip_UART_TXEnable(dev->reg_base);
}

/**
 * uart_rx_int_handler -- uart rx irq handler
 * @dev: uart device which has data available and trigger irq.
 *        This function will be called when uart device irq happened.
 *        This function will move received data to @dev->rx_queue.
 */
static void uart_rx_int_handler(struct uart_device *dev)
{
	BaseType_t xHigherProrityTaskWoken = pdFALSE;

	while(Chip_UART_ReadLineStatus(dev->reg_base) & UART_LSR_RDR) {
		uint8_t ch = Chip_UART_ReadByte(dev->reg_base);
		if (xQueueSendFromISR(dev->rx_queue, &ch,
					&xHigherProrityTaskWoken) == pdFALSE)
			break;
	}

	if (xHigherProrityTaskWoken != pdFALSE)
		taskYIELD();

}

/**
 * uart_tx_int_handler -- uart tx irq handler
 * @dev: uart device to send data.
 *        This function will be called when uart device irq happened.
 *        This function will send data from @dev->tx_queue.
 */
static void uart_tx_int_handler(struct uart_device *dev)
{
	BaseType_t xTaskWokenByReceive = pdFALSE;
	uint8_t ch;

	while((Chip_UART_ReadLineStatus(dev->reg_base) & UART_LSR_THRE) != 0 &&
			xQueueReceiveFromISR(dev->tx_queue, &ch,
				&xTaskWokenByReceive) == pdTRUE) {
		Chip_UART_SendByte(dev->reg_base, ch);
	}

	if (xTaskWokenByReceive != pdFALSE)
		taskYIELD();
}

/**
 * uart_auto_baud_int_handler -- auto baudrate interrupt service routine.
 * @dev: uart device occurs a auto baud interrupt.
 *	FIXME: This function has not implement yet.
 */
static void uart_auto_baud_int_handler(struct uart_device *dev)
{
}

/**
 * uart_irq_handler -- irq handler for each uart device
 * @dev: uart device which will use the function to handle irq.
 */
static void uart_irq_handler(struct uart_device *dev)
{
	if (dev->reg_base->IER & UART_IER_THREINT) {
		uart_tx_int_handler(dev);
		if (xQueueIsQueueEmptyFromISR(dev->tx_queue) == pdTRUE) {
			Chip_UART_IntDisable(dev->reg_base, UART_IER_THREINT);
		}
	}

	uart_rx_int_handler(dev);
	uart_auto_baud_int_handler(dev);
}

/**
 * uart_init -- initialize uart device interface
 * @dev: uart device will be initialized.
 *       Each uart device must be initialized before using.
 *       This function will be called in uart_device_register().
 */
static void uart_init(struct uart_device *dev)
{
	if (dev->initialized)
		return;

	dev->send_mutex = xSemaphoreCreateMutex();
	dev->recv_mutex = xSemaphoreCreateMutex();
	dev->tx_queue = xQueueCreate(UART_SEND_BUFFER_LEN, sizeof(uint8_t));
	dev->rx_queue = xQueueCreate(UART_RECV_BUFFER_LEN, sizeof(uint8_t));

	if (dev->send_mutex && dev->recv_mutex &&
			dev->tx_queue && dev->rx_queue) {
		/* Setup pins for dev */
		uart_set_pinctrls(dev);
		/* Default init uart controller */
		Chip_UART_Init(dev->reg_base);
		/* Set baudrate */
		uart_set_baudrate(dev, dev->baudrate);
		/* Set line control */
		uart_set_line_control(dev, dev->word_length,
				dev->stop_bits, dev->parity);

		dev->initialized = true;
	}
}

/**
 * uart_deinit -- deinitialize uart device interface
 * @dev: uart device will be deinitialized.
 *       Each uart device must be initialized if it won't be used again.
 *       This function will be called in uart_device_unregister().
 */
static void uart_deinit(struct uart_device *dev)
{
	if (!dev->initialized)
		return;

	vSemaphoreDelete(dev->send_mutex);
	vSemaphoreDelete(dev->recv_mutex);
	vQueueDelete(dev->tx_queue);
	vQueueDelete(dev->rx_queue);
	Chip_UART_DeInit(dev->reg_base);
	dev->initialized = false;
}

/**
 * uart_dev_index -- get index of a uart device
 */
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

/**
 * uart_dev_tx_queued -- send queued data in dev->tx_queue
 * @dev: uart device.
 */
static void uart_dev_tx_queued(struct uart_device *dev)
{
	uint8_t ch;

	while((Chip_UART_ReadLineStatus(dev->reg_base) & UART_LSR_THRE) != 0 &&
			xQueueReceive(dev->tx_queue, (void *)&ch, 0)) {
		Chip_UART_SendByte(dev->reg_base, ch);
	}
}

/**
 * uart_send -- send some bytes through uart device
 * @dev: uart device used to send.
 * @buf: the data will be sent.
 * @len: length of data will be sent.
 *      return length of bytes be sent or -1 if dev is not initialized.
 *      The return value may be equal or less than  @len. So the program
 *      should check how many bytes sent successfully by calling this function.
 */
int32_t uart_send(struct uart_device *dev, uint8_t *buf, int32_t len)
{
	int i = 0;

	if (!dev->initialized)
		return -1;

	if (xSemaphoreTake(dev->send_mutex, portMAX_DELAY) == pdTRUE) {
		/* Disable Uart interrupt before moving date into queue */
		/* FIXME: Is this needed when send data to a interrupt safe
		 * queue ?
		 */
		Chip_UART_IntDisable(dev->reg_base, UART_IER_THREINT);

		/* Send as much data as possible to the tx_queue of the
		 * device
		 */
		while (i < len &&
			xQueueSend(dev->tx_queue, &buf[i],
				(TickType_t) 0) == pdTRUE)
			i++;

		/* Move as much data as possible to uart fifo */
		uart_dev_tx_queued(dev);

		/* Send remained data as much as possible to the tx_queue
		 * of the uart device.
		 */
		while (i < len &&
			xQueueSend(dev->tx_queue, &buf[i],
				(TickType_t) 0) == pdTRUE)
			i++;

		/* FIXME: Is this needed when send data to a interrupt safe
		 * queue ?
		 * */
		Chip_UART_IntEnable(dev->reg_base, UART_IER_THREINT);
		xSemaphoreGive(dev->send_mutex);
	}

	return i;
}

/**
 * uart_recv -- receive some bytes from uart device
 * @dev: uart device used to receive data.
 * @buf: the data received will be store here.
 * @len: number of bytes want to receive.
 *      return length of bytes be received or -1 if dev is not
 *      initialized.
 *      The returned value may be equal or less than @len.
 *      So the program call this function should check return value.
 */
int32_t uart_recv(struct uart_device *dev, uint8_t *buf, int32_t len)
{
	int i = 0;

	if (!dev->initialized)
		return -1;

	if (xSemaphoreTake(dev->recv_mutex, portMAX_DELAY) == pdTRUE) {
		while(i < len &&
			xQueueReceive(dev->rx_queue, &buf[i],
				(TickType_t) 0) == pdTRUE)
			i++;
		xSemaphoreGive(dev->recv_mutex);
	}

	return i;
}

/**
 * uart_device_register -- register a uart device into system
 * @dev: uart device will be registered.
 *      This function will register a uart device into system
 *      and initialize its hardware.
 */
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

/**
 * uart_device_unregister -- unregister a uart device from system
 * @dev: uart device will be unregistered.
 *      This function will unregister a uart device from system
 *      and deinitialize its hardware.
 */
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

/* Configurations for UART0 */
static const PINMUX_GRP_T uart0_pinctrls[] = {
	{0, 2, (IOCON_FUNC1 | IOCON_MODE_INACT)},
	{0, 3, (IOCON_FUNC1 | IOCON_MODE_INACT)},
};

struct uart_device uart0 = {
	.baudrate = 115200,
	.word_length = WORD_LENGTH_8BITS,
	.stop_bits = ONE_STOP_BIT,
	.parity = PARITY_DISABLE,
	.reg_base = LPC_UART0,
	.pinctrls = &uart0_pinctrls,
	.pins = sizeof(uart0_pinctrls) / sizeof(uart0_pinctrls[0]),
	.irq = UART0_IRQn,
	/* irq_prior must be equal or higher than this */
	.irq_prior = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
};

/**
 * UART0_IRQHandler -- IRQ Handler for UART0
 */
void UART0_IRQHandler(void)
{
	uart_irq_handler(&uart0);
}

/**
 * uart_init_all -- register and initialize all uarts configured.
 */
void uart_init_all()
{
	uart_device_register(&uart0);
}
/* End of uart.c */
