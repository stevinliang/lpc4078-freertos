/**
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Oct 17, 2016
 *      This file is for retarget printf in newlib nano
 *      to what you want
 */
#include "retarget.h"
#include "uart.h"

void retarget_init()
{
	// Initialize UART
	uart_init_all();
}

int _write (int fd, char *ptr, int len)
{
	/* Write "len" of char from "ptr" to file id "fd"
	 * Return number of char written.
	 * Need implementing with UART here. */
	uart_send(&uart0, (uint8_t *)ptr, len);

	return len;
}

int _read (int fd, char *ptr, int len)
{
	/* Read "len" of char to "ptr" from file id "fd"
	 * Return number of char read.
	 * Need implementing with UART here. */
	return len;
}

void _ttywrch(int ch) {
	/* Write one char "ch" to the default console
	 * Need implementing with UART here. */
	uart_send(&uart0, (uint8_t *)&ch, 1);
}

/* SystemInit will be called before main */
/*
void SystemInit()
{
	retarget_init();
}
*/

