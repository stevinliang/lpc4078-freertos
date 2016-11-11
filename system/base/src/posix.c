/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 10, 2016
 *	posix abstract layer for userspace.
 *
 * History:
 **/
#include "posix.h"
#include "base.h"
#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "string.h"
#include "device.h"
#include "stdarg.h"


#define STD_IN_DEV  "/uart0"
#define STD_OUT_DEV "/uart0"
#define STD_ERR_DEV "/uart0"

/**
 * array list to keep system file descriptor.
 */
static struct file_desc sys_fds[MAX_FD_SUPPORT];

/**
 * get_free_fd -- find an available file descriptor slot in sys_fds.
 */
static int get_free_fd(void)
{
	int i;

	for (i = 0; i < MAX_FD_SUPPORT; i++) {
		if (sys_fds[i].in_use != true)
			return i;
	}

	return -EINVAL;
}

/**
 * open - open a device that registered to the system.
 * @pathname: the name of the device with prefix of '/', such as "/uart0",
 *		"/spi0"
 * @flags: O_NOBLOCK, O_RDONLY, O_WRONLY, O_RDWR could be used. This args do not
 *	   effect any thing. This arg is just to compatible with posix
 *	   interface.
 * @mode: NOT USED.
 *         return a nonegtive integer if success, negtive integer if failed.
 *         The returned nonegtive integer is the file descriptor.
 */
int open(const char *pathname, ...)
{
	int fd;
	int ret = -EINVAL;
	int flags = 0, mode = 0;
	struct device *dev;

	va_list va;
	va_start(va, pathname);
	flags = va_arg(va, int);
	mode = va_arg(va, int);
	va_end(va);

	taskENTER_CRITICAL();
	dev = get_device_by_name(pathname);

	if (dev == NULL) {
		ret = -EEXIST;
	} else {

		if (dev->ops->open)
			ret = dev->ops->open(dev, flags);
		if (!ret) {
			fd = get_free_fd();
			if (fd  >= 0) {
				sys_fds[fd].dev = dev;
				sys_fds[fd].in_use = true;
				sys_fds[fd].flags = flags;
				sys_fds[fd].mode = mode;
				ret = fd;
			}
		}
	}
	taskEXIT_CRITICAL();

	return ret;
}

/**
 * close - close a device that opened before.
 * @fd: file descriptor returned by open().
 *      return 0 if success. negtive integer if failed.
 */
int close(int fd)
{
	struct device *dev;

	taskENTER_CRITICAL();
	dev = sys_fds[fd].dev;
	sys_fds[fd].in_use = false;

	if (dev && dev->ops->close)
		return dev->ops->close(dev);
	taskEXIT_CRITICAL();

	return 0;
}

int read(int fd, void *buf, int count)
{
	struct device *dev;

	dev = sys_fds[fd].dev;

	if (dev && dev->ops->read)
		return dev->ops->read(dev, buf, count);

	return -EINVAL;
}

int write(int fd, const void *buf, int count)
{
	struct device *dev;

	dev = sys_fds[fd].dev;

	if (dev && dev->ops->read)
		return dev->ops->write(dev, buf, count);

	return -EINVAL;
}

void init_posix_subsystem(void)
{
	memset(sys_fds, 0, ARRAY_SIZE(sys_fds));

	sys_fds[0].dev = get_device_by_name(STD_IN_DEV);
	sys_fds[0].in_use = true;

	if (sys_fds[0].dev)
		sys_fds[0].dev->ops->open(sys_fds[0].dev, 0);

	sys_fds[1].dev = get_device_by_name(STD_OUT_DEV);
	sys_fds[1].in_use = true;
	if (sys_fds[1].dev)
		sys_fds[1].dev->ops->open(sys_fds[1].dev, 0);

	sys_fds[2].dev = get_device_by_name(STD_ERR_DEV);
	sys_fds[2].in_use = true;
	if (sys_fds[2].dev)
		sys_fds[2].dev->ops->open(sys_fds[2].dev, 0);
}

