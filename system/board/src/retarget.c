/**
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Oct 17, 2016
 *      This file is for retarget printf in newlib nano
 *      to what you want
 */
#include "board_api.h"
#include "board.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "retarget.h"
#include "uart.h"

#ifndef RETARGET_UART
	#define RETARGET_UART (&uart0)
#endif

void retarget_init()
{
	// Initialize UART
	uart_device_register(RETARGET_UART);
}

int _write (int fd, char *ptr, int len)
{
	/* Write "len" of char from "ptr" to file id "fd"
	 * Return number of char written.
	 * Need implementing with UART here. */
	return uart_send(RETARGET_UART, (uint8_t *)ptr, len);
}

int _read (int fd, char *ptr, int len)
{
	/* Read "len" of char to "ptr" from file id "fd"
	 * Return number of char read.
	 * Need implementing with UART here. */
	return uart_recv(RETARGET_UART, (uint8_t *)ptr, len);
}

void _ttywrch(int ch) {
	/* Write one char "ch" to the default console
	 * Need implementing with UART here. */
	uart_send(RETARGET_UART, (uint8_t *)&ch, 1);
}

/* SystemInit will be called before main */
/*
void SystemInit()
{
	retarget_init();
}
*/

