/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 10, 2016
 *	posix abstract layer for userspace.
 *
 * History:
 **/
#ifndef __POSIX_H
#define __POSIX_H

#include <stdio.h>
#include "base.h"
#include "chip.h"

#define MAX_FD_SUPPORT     256

#define O_NOBLOCK	(1UL << 0)
#define O_RDONLY	(1UL << 1)
#define O_WRONLY	(1UL << 2)
#define O_RDWR		(O_RDONLY | O_WRONLY)

struct file_desc {
	struct device *dev;
	int flags;
	int mode;
	bool in_use;
};

int open(const char *pathname, ...);
int close(int fd);

int read(int fd, void *buf, int count);
int write(int fd, const void *buf, int count);

void init_posix_subsystem(void);

#endif /* ifndef __POSIX_H */
