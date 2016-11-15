/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 10, 2016
 *	device driver subsystem.
 **/
#include "device.h"
#include "base.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "uart.h"
#include "spi_slave.h"
#define MAX_DEVICES_SUPPORT 100
/* Keep all registerd devices */
static struct device *sys_devices[MAX_DEVICES_SUPPORT];

struct device *get_device_by_name(const char *name)
{
	int i;

	for (i = 0; i < MAX_DEVICES_SUPPORT; i++) {
		if (sys_devices[i] != NULL &&
				!strcmp(name, sys_devices[i]->name))
			return sys_devices[i];
	}

	return NULL;
}

/**
 * register_device -- register a device to the system
 * @dev: device will be registered.
 *       return 0 if success, negtive integer failed.
 */
int register_device(struct device *dev)
{
	int i;
	int ret = 0;

	if (!dev)
		return -EINVAL;
	if (!dev->ops)
		return -EINVAL;

	if (!dev->ops->read || !dev->ops->write)
		return -EINVAL;

	taskENTER_CRITICAL();
	if (get_device_by_name(dev->name) != NULL) {
		ret = -EINVAL;
	} else {
		for (i = 0; i < MAX_DEVICES_SUPPORT; i++) {
			if (sys_devices[i] == NULL) {
				sys_devices[i] = dev;
				if (dev->ops->init)
					ret = dev->ops->init(dev);
				break;
			}
		}
	}

	taskENTER_CRITICAL();
	return ret;
}

int init_device_subsystem()
{
	memset(sys_devices, 0, ARRAY_SIZE(sys_devices));

	return 0;
}

void register_all_devices()
{
	uart_init_all();
	spi_init_all();
}

/* End of device.c */
