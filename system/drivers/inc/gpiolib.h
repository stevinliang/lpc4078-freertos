/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 16, 2016
 *	gpiolib for lpc40xx device.
 *
 * History:
 **/

#ifndef _GPIOLIB_H
#define _GPIOLIB_H
#include "chip.h"

int gpio_direction_input(uint32_t gpio);
int gpio_direction_output(uint32_t gpio, int value);

int gpio_get_value(uint32_t gpio);
int gpio_set_value(uint32_t gpio, int value);

int gpio_toggle(uint32_t gpio);

#endif  /* ifndef _GPIOLIB_H */
