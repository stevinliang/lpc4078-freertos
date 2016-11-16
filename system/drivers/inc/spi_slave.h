/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 8, 2016
 *	spi_slave.h head file for spi slave driver.
 **/

#ifndef _SPI_SLAVE_H
#define _SPI_SLAVE_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "dmaengine.h"
#include "device.h"
#include "chip.h"

#define SPI_CLK_MODE0   0       /* spi mode 0 */
#define SPI_CLK_MODE1   1       /* spi mode 1 */
#define SPI_CLK_MODE2   2       /* spi mode 2 */
#define SPI_CLK_MODE3   3       /* spi mode 3 */

#define SPI_BITS_4      4
#define SPI_BITS_5      5
#define SPI_BITS_6      6
#define SPI_BITS_7      7
#define SPI_BITS_8      8
#define SPI_BITS_9      9
#define SPI_BITS_10     10
#define SPI_BITS_11     11
#define SPI_BITS_12     12
#define SPI_BITS_13     13
#define SPI_BITS_14     14
#define SPI_BITS_15     15
#define SPI_BITS_16     16

/* typedef Chip_SSP_DATA_SETUP_T spi_transfer; */

struct spi_device {
	struct device dev;
	uint8_t bits_per_word;
	uint8_t clk_mode;
	struct dma_chan dma_rx_chan;
	struct dma_chan dma_tx_chan;
	const PINMUX_GRP_T *pinctrls;
	uint32_t pins;
	uint32_t int_pin;
	LPC_SSP_T *reg_base;
	SemaphoreHandle_t mutex;
	bool initialized;
	bool is_opened;
};

struct spi_transfer {
	void *rx_buf;
	const void *tx_buf;
	int len;
};

int spi_device_register(struct spi_device *spidev);
void spi_init_all(void);





#endif /* ifndef _SPI_SLAVE_H */

