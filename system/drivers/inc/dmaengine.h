/**
 * Copyright (c) 2016, ZHENGZHOU YUTONG BUS CO,.LTD.
 * Author: Stevin Liang <liangzl@yutong.com>
 * Date: Nov 14, 2016
 *	dma engine subsystem head file.
 **/
#ifndef _DMAENGINE_H
#define _DMAENGINE_H
#include "FreeRTOS.h"
#include "semphr.h"
#include "base.h"
#include "chip.h"

#define DMA_CHANNELS 16

#define DMA_MEMORY_TO_PERIPHERAL       1
#define DMA_PERIPERAL_TO_MEMORY        2
#define DMA_MEMORY_TO_MEMORY           3

typedef enum dma_status {
	DMA_OK = 0,
	DMA_ERROR = !DMA_OK,
} dma_status_t;

struct dma_chan {
	uint8_t channel;
	uint8_t dma_req;
	uint32_t src_addr;
	uint32_t dst_addr;
	int direction;
	int len;
	dma_status_t status;
	SemaphoreHandle_t done;
};

typedef void (*dma_irq_callback_t) (void *args);

int dmaengine_channel_completion_init(struct dma_chan *chan);

int dmaengine_request_channel(struct dma_chan *chan);

int dmaengine_register_irq_callback(dma_irq_callback_t call_back, void *args);

int dmaengine_transfer(struct dma_chan *chan);

int dmaengine_channel_interrupt(struct dma_chan *chan);

void dmaengine_wait_transfer_done(struct dma_chan *chan);

void init_dmaegine_subsystem(void);


#endif /* ifndef _DMAENGINE_H */

/* End of dmaengine.h */
