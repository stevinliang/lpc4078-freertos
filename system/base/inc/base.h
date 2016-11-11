/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 8, 2016
 *	base.h head file for exporting base macros and interfaces.
 **/

#ifndef _BASE_H
#define _BASE_H
/* get array size */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define EINVAL   1
#define EBUSY    2
#define EEXIST   3
#define EAGAIN   4

#endif /* ifndef _BASE_H */
