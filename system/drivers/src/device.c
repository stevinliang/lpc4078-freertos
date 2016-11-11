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

	if (!dev)
		return -EINVAL;
	if (!dev->ops)
		return -EINVAL;

	if (!dev->ops->read || !dev->ops->write)
		return -EINVAL;

	taskENTER_CRITICAL();
	if (get_device_by_name(dev->name) != NULL)
		return -EINVAL;

	for (i = 0; i < MAX_DEVICES_SUPPORT; i++) {
		if (sys_devices[i] == NULL) {
			sys_devices[i] = dev;
			if (dev->ops->init)
				dev->ops->init(dev);
			taskENTER_CRITICAL();
			return 0;
		}
	}

	taskENTER_CRITICAL();
	return -EINVAL;
}

int init_device_subsystem()
{
	memset(sys_devices, 0, ARRAY_SIZE(sys_devices));

	return 0;
}

void register_all_devices()
{
	uart_init_all();
}

/* End of device.c */
