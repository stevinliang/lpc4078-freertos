/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 14, 2016
 *	dma engine subsystem.
 **/
#include "dmaengine.h"
#include "task.h"
#include "string.h"

struct dma_irq_callback {
	dma_irq_callback_t irq_callback;
	void *args;
};

/* For store dma irq handler call backs */
static struct dma_irq_callback dma_irq_handlers[DMA_CHANNELS];

int dmaengine_channel_completion_init(struct dma_chan *chan)
{
	chan->done = xSemaphoreCreateBinary();
	if (chan->done) {
		xSemaphoreTake(chan->done, 0);
		return 0;
	} else {
		return -ENOMEM;
	}
}

/**
 * dmaengine_request_channel - request a free dma channel.
 * @chan: information of requested channel will store here.
 */
int dmaengine_request_channel(struct dma_chan *chan)
{
	taskENTER_CRITICAL();
	chan->channel = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, chan->dma_req);
	taskEXIT_CRITICAL();
	return 0;
}

/**
 * dmaengine_register_irq_callback - register a dma irq handler
 * @call_back: irq handler call back
 *		return 0 if success
 */
int dmaengine_register_irq_callback(dma_irq_callback_t callback, void *args)
{
	int i = 0;
	int ret = -EINVAL;

	taskENTER_CRITICAL();
	for (i = 0; i < DMA_CHANNELS; i++) {
		if (!dma_irq_handlers[i].irq_callback) {
			dma_irq_handlers[i].irq_callback = callback;
			dma_irq_handlers[i].args = args;
			ret = 0;
			break;
		}
	}
	taskEXIT_CRITICAL();

	return ret;
}

/**
 * dmaengine_transfer - dmaengine transfer some data
 * @chan: use which channel to transfer data. chan store
 *	  all transfer information needed by dmaengine
 */
int dmaengine_transfer(struct dma_chan *chan)
{
	int ret = 0;

	GPDMA_FLOW_CONTROL_T flow;

	xSemaphoreTake(chan->done, 0);

	switch(chan->direction) {
	case DMA_MEMORY_TO_PERIPHERAL:
		flow = GPDMA_TRANSFERTYPE_M2P_CONTROLLER_DMA;
		break;

	case DMA_PERIPERAL_TO_MEMORY:
		flow = GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA;
		break;

	case DMA_MEMORY_TO_MEMORY:
		flow = GPDMA_TRANSFERTYPE_M2M_CONTROLLER_DMA;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret)
		return ret;

	taskENTER_CRITICAL();
	ret = Chip_GPDMA_Transfer(LPC_GPDMA, chan->channel,
			(uint32_t)chan->src_addr,
			chan->dst_addr,
			chan->direction,
			chan->len);
	if (ret == SUCCESS)
		ret = 0;
	else
		ret = -EIO;
	taskEXIT_CRITICAL();

	return ret;
}

/**
 * dmaengine_wait_transfer_done - wait for last transfer done.
 * @chan: which dma channel waited for
 */
void dmaengine_wait_transfer_done(struct dma_chan *chan)
{
	xSemaphoreTake(chan->done, portMAX_DELAY);
}

/**
 * dmaengine_channel_interrupt - checking whether channel Interrupt
 *	happened.  clear it if channel interrupt pending.
 *@chan: dma channel will be checked.
 */
int dmaengine_channel_interrupt(struct dma_chan *chan)
{
	if (Chip_GPDMA_Interrupt(LPC_GPDMA, chan->channel))
		return 0;
	else
		return -EINVAL;
}

void DMA_IRQHandler(void)
{
	int i;
	struct dma_irq_callback *irq_func = NULL;

	for (i = 0; i < DMA_CHANNELS; i++) {
		irq_func = &dma_irq_handlers[i];
		if ( irq_func->irq_callback)
			irq_func->irq_callback(irq_func->args);
	}
}

void dmaegine_init_all(void)
{
	memset(dma_irq_handlers, 0, ARRAY_SIZE(dma_irq_handlers));
	Chip_GPDMA_Init(LPC_GPDMA);
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn,
			configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(DMA_IRQn);
}

/* End of dmaengine.c */
