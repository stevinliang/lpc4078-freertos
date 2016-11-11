/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 10, 2016
 *	Head file for device abstract layer.
 **/
#ifndef __DEVICE_H
#define __DEVICE_H
#include "chip.h"

struct device {
	char name[48];
	void *pri_data;
	struct dev_ops *ops;
	int flags;
	bool opened;
};

struct dev_ops {
	int (*init)(struct device *dev);
	int (*open)(struct device *dev, int flags);
	int (*close)(struct device *dev);
	int (*read)(struct device *dev, void *buf, int count);
	int (*write)(struct device *dev, const void *buf, int count);
};

struct device *get_device_by_name(const char *name);

int register_device(struct device *dev);

void register_all_devices(void);
int init_device_subsystem(void);

#endif /* ifndef __DEVICE_H */
