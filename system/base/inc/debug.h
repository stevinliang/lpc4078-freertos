/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 8, 2016
 *	debug.h head file for exporting debug macros and interfaces.
 **/
#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG
#include <stdio.h>
	#define debug(...)  printf(__VA_ARGS__)
#else
	#define debug(...)
#endif /* ifdef DEBUG */

#endif /* ifndef _DEBUG_H */
