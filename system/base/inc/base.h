/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 8, 2016
 *	base.h head file for exporting base macros and interfaces.
 **/

#ifndef _BASE_H
#define _BASE_H
#include "FreeRTOS.h"
#include "errno.h"
/* get array size */
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))
#define ASSERT(x)	configASSERT(x)

#endif /* ifndef _BASE_H */
