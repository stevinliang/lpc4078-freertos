/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 16, 2016
 *	gpiolib for lpc40xx device.
 *
 * History:
 **/
#include "gpiolib.h"

#define GPIO_PORT(x) ((x >> 5) & 0xFF)
#define GPIO_PIN(x)  (x & 0x1F)


/***********************************************************
 * gpio = portNum * 32 + pinNum; For example:
 * P0[1]: gpio = 0 * 32 + 1 = 1;
 * p2[2]: gpio = 2 * 32 + 2 = 66;
 **/

/**
 * gpio_direction_input -- set gpio direction input.
 * @gpio: gpio used.
 */
int gpio_direction_input(uint32_t gpio)
{
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio), false);
	return 0;
}

/**
 * gpio_direction_output -- set gpio direction output with default out value.
 * @gpio: gpio used.
 * @value: output value to set.
 */
int gpio_direction_output(uint32_t gpio, int value)
{
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio), true);
	if (value == 0)
		Chip_GPIO_SetPinOutLow(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio));
	else
		Chip_GPIO_SetPinOutHigh(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio));

	return 0;
}

/**
 * gpio_get_value -- get a gpio pin value.
 * @gpio: gpio used.
 *        return gpio value. high return 1, low retun 0.
 */
int gpio_get_value(uint32_t gpio)
{
	if (Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio)))
		return 1;
	else
		return 0;
}

/**
 * gpio_set_value -- set a gpio pin value.
 * @gpio: gpio used.
 * @value: to set low value = 0, set high value = !0.
 */
int gpio_set_value(uint32_t gpio, int value)
{
	if (value == 0)
		Chip_GPIO_SetPinOutLow(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio));
	else
		Chip_GPIO_SetPinOutHigh(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio));

	return 0;
}

/**
 * gpio_toggle -- toggle a gpio output state.
 * @gpio: gpio will be toggled.
 */
int gpio_toggle(uint32_t gpio)
{
	Chip_GPIO_SetPinToggle(LPC_GPIO, GPIO_PORT(gpio), GPIO_PIN(gpio));
	return 0;
}

/* End of gpiolib.c */
