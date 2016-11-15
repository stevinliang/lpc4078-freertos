/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 8, 2016
 *	spi slave driver for lpc40xx device.
 *
 * History:
 *	Nov 8, 2016: create this file.
 **/

#include "spi_slave.h"
#include "task.h"
#include "base.h"

#define SPI_PORT_NUM 3
#define SPI_MAX_TRANSFER_LEN 256

static uint8_t dummy[SPI_MAX_TRANSFER_LEN];

static struct spi_device *spi_devices[SPI_PORT_NUM];

/**
 * spi_device_set_pinctrls -- configure pins which spi device used.
 * @spidev: pointer to a spi_device structure.
 */
static void spi_device_set_pinctrls(struct spi_device *spidev)
{
	Chip_IOCON_SetPinMuxing(LPC_IOCON, spidev->pinctrls, spidev->pins);
}

/**
 * spi_device_set_format -- setup spi format
 * @spidev: which spi device setup spi format for
 */
static void spi_device_set_format(struct spi_device *spidev)
{
	CHIP_SSP_BITS_T bits;
	CHIP_SSP_CLOCK_MODE_T clock_mode;

	switch (spidev->bits_per_word) {
	case SPI_BITS_4:
		bits = SSP_BITS_4;
		break;

	case SPI_BITS_5:
		bits = SSP_BITS_5;
		break;

	case SPI_BITS_6:
		bits = SSP_BITS_6;
		break;

	case SPI_BITS_7:
		bits = SSP_BITS_7;
		break;

	case SPI_BITS_8:
		bits = SSP_BITS_8;
		break;

	case SPI_BITS_9:
		bits = SSP_BITS_9;
		break;

	case SPI_BITS_10:
		bits = SSP_BITS_10;
		break;

	case SPI_BITS_11:
		bits = SSP_BITS_11;
		break;

	case SPI_BITS_12:
		bits = SSP_BITS_12;
		break;

	case SPI_BITS_13:
		bits = SSP_BITS_13;
		break;

	case SPI_BITS_14:
		bits = SSP_BITS_14;
		break;

	case SPI_BITS_15:
		bits = SSP_BITS_15;
		break;

	case SPI_BITS_16:
		bits = SSP_BITS_16;
		break;

	default:
		bits = SSP_BITS_8;
		break;
	}

	switch (spidev->clk_mode) {
	case SPI_CLK_MODE0:
		clock_mode = SSP_CLOCK_MODE0;
		break;

	case SPI_CLK_MODE1:
		clock_mode = SSP_CLOCK_MODE1;
		break;

	case SPI_CLK_MODE2:
		clock_mode = SSP_CLOCK_MODE2;
		break;

	case SPI_CLK_MODE3:
		clock_mode = SSP_CLOCK_MODE3;
		break;

	default:
		clock_mode = SSP_CLOCK_MODE0;
		break;
	}

	Chip_SSP_SetFormat(spidev->reg_base, bits,
			SSP_FRAMEFORMAT_SPI, clock_mode);
}

/**
 * spi_device_index -- get spi port index
 * @spidev: spi slave
 *          return index of a spi slave.
 */
static int spi_device_index(struct spi_device *spidev)
{
	int index = -1;

	switch ((uint32_t)spidev->reg_base) {
	case (uint32_t)LPC_SSP0:
		index = 0;
		break;

	case (uint32_t)LPC_SSP1:
		index = 1;
		break;

	case (uint32_t)LPC_SSP2:
		index = 2;
		break;

	default:
		break;
	}

	return index;
}

/**
 * spi_device_init -- init spi device
 * @spidev: spi device will be initialized.
 */
static int spi_device_init(struct spi_device *spidev)
{
	if (spidev->initialized)
		return -EEXIST;

	spidev->mutex = xSemaphoreCreateMutex();
	if (spidev->mutex) {
		spi_device_set_pinctrls(spidev);
		Chip_SSP_Init(spidev->reg_base);
		Chip_SSP_SetMaster(spidev->reg_base, 0);
		spi_device_set_format(spidev);
		spidev->initialized = true;
		return 0;
	} else {
		return -ENOMEM;
	}
}

/**
 * spi_device_enable -- enable spi device
 * @spidev: spi device will be enabled.
 */
static void spi_device_enable(struct spi_device *spidev)
{
	Chip_SSP_DMA_Enable(spidev->reg_base);
	Chip_SSP_Enable(spidev->reg_base);
}

/**
 * spi_device_disable -- disable spi device
 * @spidev: spi device will be disabled.
 */
static void spi_device_disable(struct spi_device *spidev)
{
	Chip_SSP_Disable(spidev->reg_base);
	Chip_SSP_DMA_Disable(spidev->reg_base);
}

/**
 * spi_device_transfer -- transfer some data through spi.
 * @spidev: spi device will be used to transfer data.
 * @xfer: one spi data transfer.
 *	return data length be transfered successfully.
 *	Or negtive integer for error.
 */
static int spi_device_transfer(struct spi_device *spidev,
		struct spi_transfer *xfer)
{
	if ((xfer->tx_buf == NULL) || (xfer->rx_buf == NULL))
		return -EINVAL;

	spidev->dma_tx_chan.src_addr = (uint32_t)xfer->tx_buf;
	spidev->dma_tx_chan.len = xfer->len;
	spidev->dma_tx_chan.status = DMA_OK;

	spidev->dma_rx_chan.dst_addr = (uint32_t)xfer->rx_buf;
	spidev->dma_rx_chan.len = xfer->len;
	spidev->dma_rx_chan.status = DMA_OK;

	if (dmaengine_transfer(&spidev->dma_tx_chan) ||
			dmaengine_transfer(&spidev->dma_rx_chan)) {
		return -EIO;
	}

	dmaengine_wait_transfer_done(&spidev->dma_tx_chan);
	dmaengine_wait_transfer_done(&spidev->dma_rx_chan);

	if ((!spidev->dma_tx_chan.status) && (!spidev->dma_rx_chan.status)) {
		return xfer->len;
	} else {
		return -EIO;
	}
}

/**
 * spi_dev_dma_irq_handler -- spi dma irq handler callback
 * @args: spidev
 */
static void spi_dev_dma_irq_handler(void *args)
{
	struct spi_device *spidev = (struct spi_device *)args;
	BaseType_t xTaskWoken = pdFALSE;

	if (dmaengine_channel_interrupt(&spidev->dma_tx_chan)) {
		xSemaphoreGiveFromISR(spidev->dma_tx_chan.done, &xTaskWoken);
		spidev->dma_tx_chan.status = DMA_OK;
	}

	if (dmaengine_channel_interrupt(&spidev->dma_rx_chan)) {
		xSemaphoreGiveFromISR(spidev->dma_rx_chan.done, &xTaskWoken);
		spidev->dma_rx_chan.status = DMA_OK;
	}

	if (xTaskWoken != pdFALSE)
		taskYIELD();
}

/**
 * spi_device_register -- register a spi device to the system.
 * @dev: spi device will be registered.
 *       return -1 if the dev has been registerd or unknown spi port.
 *       return 0 register success.
 */
int spi_device_register(struct spi_device *spidev)
{
	int index = spi_device_index(spidev);

	if (index < 0)
		return -EINVAL;

	if (spi_devices[index])
		return -EINVAL;

	spi_devices[index] = spidev;
	spidev->dev.pri_data = spidev;

	return register_device(&spidev->dev);
}

/**
 * spi_init -- init registered device.
 * @dev: device will be initialized.
 *	return 0 if successful, negtive integer if failed.
 */
static int spi_init(struct device *dev)
{
	int ret = 0;
	struct spi_device *spidev = (struct spi_device *)dev->pri_data;

	ret = spi_device_init(spidev);
	if (ret)
		return ret;

	ret = (dmaengine_channel_completion_init(&spidev->dma_tx_chan) ||
		dmaengine_channel_completion_init(&spidev->dma_tx_chan) ||
		dmaengine_request_channel(&spidev->dma_tx_chan) ||
		dmaengine_request_channel(&spidev->dma_rx_chan) ||
		dmaengine_register_irq_callback(spi_dev_dma_irq_handler,
				(void *)spidev));

	return ret;
}

/**
 * spi_open -- open a spi device registerd to the system.
 * @flags: open flags.
 *	return 0 if successful, negtive if failed.
 */
static int spi_open(struct device *dev, int flags)
{
	struct spi_device *spidev = (struct spi_device *)dev->pri_data;
	if (spidev->initialized)
		return -EINVAL;
	if (spidev->is_opened)
		return -EBUSY;

	dev->flags = flags;
	spi_device_enable(spidev);

	return 0;
}

/**
 * spi_close -- close a opened spi device.
 * @dev: spi device will be closed.
 *	return 0 successful
 */
static int spi_close(struct device *dev)
{
	struct spi_device *spidev = (struct spi_device *)dev->pri_data;
	dev->flags = 0;
	spidev->is_opened = false;
	spi_device_disable(spidev);

	return 0;
}

static int spi_write(struct device *dev, const void *buf, int len)
{
	struct spi_device *spidev = (struct spi_device *)dev->pri_data;
	struct spi_transfer xfer;
	uint8_t *tmp = (uint8_t *)buf;
	int remain_bytes = len;
	int once_xfered;

	if (xSemaphoreTake(spidev->mutex, portMAX_DELAY) == pdTRUE) {
		while (remain_bytes) {
			xfer.len = remain_bytes > SPI_MAX_TRANSFER_LEN
				? SPI_MAX_TRANSFER_LEN
				: remain_bytes;
			xfer.tx_buf = (const void *)tmp;
			tmp += xfer.len;
			xfer.rx_buf = (void *)dummy;
			once_xfered = spi_device_transfer(spidev, &xfer);
			if (once_xfered > 0 && once_xfered < xfer.len) {
				remain_bytes -= once_xfered;
				break;
			} else if (once_xfered < 0) {
				break;
			}
			remain_bytes -= xfer.len;
		}

		xSemaphoreGive(spidev->mutex);
	}

	return (len - remain_bytes);
}

static int spi_read(struct device *dev, void *buf, int len)
{
	struct spi_device *spidev = (struct spi_device *)dev->pri_data;
	struct spi_transfer xfer;
	uint8_t *tmp = (uint8_t *)buf;
	int remain_bytes = len;
	int once_xfered;

	if (xSemaphoreTake(spidev->mutex, portMAX_DELAY) == pdTRUE) {
		while (remain_bytes) {
			xfer.len = remain_bytes > SPI_MAX_TRANSFER_LEN
				? SPI_MAX_TRANSFER_LEN : remain_bytes;
			xfer.rx_buf = (void *)tmp;
			tmp += xfer.len;
			xfer.tx_buf = (const void *)dummy;
			once_xfered = spi_device_transfer(spidev, &xfer);
			if (once_xfered > 0 && once_xfered < xfer.len) {
				remain_bytes -= once_xfered;
				break;
			} else if (once_xfered < 0) {
				break;
			}

			remain_bytes -= xfer.len;
		}
		xSemaphoreGive(spidev->mutex);
	}

	return (len - remain_bytes);
}

static struct dev_ops spi_ops = {
	.open = spi_open,
	.close = spi_close,
	.read = spi_read,
	.write = spi_write,
	.init = spi_init,
};

static struct spi_device spi0 = {
	.dev = {
		.name = "/spi0",
		.ops = &spi_ops,
	},
	.dma_tx_chan = {
		.dst_addr = GPDMA_CONN_SSP0_Tx,
		.direction = DMA_MEMORY_TO_PERIPHERAL,
	},
	.dma_rx_chan = {
		.src_addr = GPDMA_CONN_SSP0_Rx,
		.direction = DMA_PERIPERAL_TO_MEMORY,
	},
};

void spi_init_all(void)
{
	spi_device_register(&spi0);
}


/* End of spi_slave.c */
