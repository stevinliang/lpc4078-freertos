/**
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Oct 17, 2016
 *      This file is for retarget printf in newlib nano
 *      to what you want
 */
#include "retarget.h"
#include "posix.h"

void retarget_init()
{
	// Initialize UART
}

int _write (int fd, char *ptr, int len)
{
	/* Write "len" of char from "ptr" to file id "fd"
	 * Return number of char written.
	 * Need implementing with UART here. */
	return write(fd, (const void *)ptr, len);
}

int _read (int fd, char *ptr, int len)
{
	/* Read "len" of char to "ptr" from file id "fd"
	 * Return number of char read.
	 * Need implementing with UART here. */
	return read(fd, (void *)ptr, len);
}

void _ttywrch(int ch) {
	/* Write one char "ch" to the default console
	 * Need implementing with UART here. */
	write(1, (const void*)&ch, 1);
}

/* SystemInit will be called before main */
/*
void SystemInit()
{
	retarget_init();
}
*/

