/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Lucian Copeland for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "peripherals/dma.h"

static DMA_HandleTypeDef * dma1_handles[8];
static DMA_HandleTypeDef * dma2_handles[8];

void dma_reset(void) {
    for (int i = 0; i < 8; i++) {
        dma1_handles[i] = NULL;
        dma2_handles[i] = NULL;
    }
}

bool stm32_peripherals_dma_init(DMA_HandleTypeDef* handle, uint8_t dma_index, uint8_t stream_idx,
                                    uint32_t channel, uint32_t direction, uint32_t priority, uint32_t subprio) {

    DMA_Stream_TypeDef* stream;
    if (dma_index == 1) {
        __HAL_RCC_DMA1_CLK_ENABLE();
        stream = (DMA_Stream_TypeDef*)dma1_stream_map[stream_idx];
        dma1_handles[stream_idx] = handle;
    } else if (dma_index == 2) {
        __HAL_RCC_DMA2_CLK_ENABLE();
        stream = (DMA_Stream_TypeDef*)dma2_stream_map[stream_idx];
        dma2_handles[stream_idx] = handle;
    } else {
        return false;
    }

    handle->Instance                 = stream;
    handle->Init.Channel             = channel;
    handle->Init.Direction           = direction;
    handle->Init.PeriphInc           = DMA_PINC_DISABLE;
    handle->Init.MemInc              = DMA_MINC_ENABLE;
    handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    handle->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    handle->Init.Mode                = DMA_NORMAL;
    handle->Init.Priority            = priority;
    handle->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    handle->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    handle->Init.MemBurst            = DMA_MBURST_INC4;
    handle->Init.PeriphBurst         = DMA_PBURST_INC4;
    if (HAL_DMA_Init(handle) != HAL_OK) {
        return false;
    }

    if (dma_index == 1) {
        HAL_NVIC_SetPriority(dma1_irq_map[stream_idx], 0, subprio);
        HAL_NVIC_EnableIRQ(dma1_irq_map[stream_idx]);
    } else if (dma_index == 2) {
        HAL_NVIC_SetPriority(dma2_irq_map[stream_idx], 0, subprio);
        HAL_NVIC_EnableIRQ(dma2_irq_map[stream_idx]);
    }

    return true;
}

bool stm32_peripherals_dma_get_params(dma_channel_type dma_type, uint8_t periph_idx,
                                      uint8_t* dma_idx, uint8_t* stream_idx, uint32_t* channel) {
    // Iterate through DMA channel options until one matches the desired peripheral (SPI1_TX, etc)
    // TODO: implement reservation system
    for (size_t i = 0; i < (sizeof(dma_map)/sizeof(*dma_map)); i++) {
        if (dma_map[i].type == dma_type && periph_idx == dma_map[i].periph_index) {
            *dma_idx = dma_map[i].dma_index;
            *stream_idx = dma_map[i].stream_index;
            *channel = dma_map[i].channel;
            return true;
        }
    }
    // No matching options found.
    return false;
}

// Only call handles if they exist, else ignore
#define DMA_IRQ_CALLHANDLE(arr, idx) if (arr[idx]) HAL_DMA_IRQHandler(arr[idx]);

// void DMA1_Stream0_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 0);
// }
// void DMA1_Stream1_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 1);
// }
// void DMA1_Stream2_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 2);
// }
// void DMA1_Stream3_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 3);
// }
// void DMA1_Stream4_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 4);
// }
// void DMA1_Stream5_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 5);
// }
// void DMA1_Stream6_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 6);
// }
// void DMA1_Stream7_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma1_handles, 7);
// }

// void DMA2_Stream0_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 0);
// }
// void DMA2_Stream1_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 1);
// }
// void DMA2_Stream2_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 2);
// }
// void DMA2_Stream3_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 3);
// }
// void DMA2_Stream4_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 4);
// }
// void DMA2_Stream5_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 5);
// }
// void DMA2_Stream6_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 6);
// }
// void DMA2_Stream7_IRQHandler(void) {
//     DMA_IRQ_CALLHANDLE(dma2_handles, 7);
// }

const IRQn_Type dma1_irq_map[8] = {
    DMA1_Stream0_IRQn,
    DMA1_Stream1_IRQn,
    DMA1_Stream2_IRQn,
    DMA1_Stream3_IRQn,
    DMA1_Stream4_IRQn,
    DMA1_Stream5_IRQn,
    DMA1_Stream6_IRQn,
    DMA1_Stream7_IRQn,
};

const IRQn_Type dma2_irq_map[8] = {
    DMA2_Stream0_IRQn,
    DMA2_Stream1_IRQn,
    DMA2_Stream2_IRQn,
    DMA2_Stream3_IRQn,
    DMA2_Stream4_IRQn,
    DMA2_Stream5_IRQn,
    DMA2_Stream6_IRQn,
    DMA2_Stream7_IRQn,
};

const DMA_Stream_TypeDef* dma1_stream_map[8] = {
    DMA1_Stream0,
    DMA1_Stream1,
    DMA1_Stream2,
    DMA1_Stream3,
    DMA1_Stream4,
    DMA1_Stream5,
    DMA1_Stream6,
    DMA1_Stream7,
};

const DMA_Stream_TypeDef* dma2_stream_map[8] = {
    DMA2_Stream0,
    DMA2_Stream1,
    DMA2_Stream2,
    DMA2_Stream3,
    DMA2_Stream4,
    DMA2_Stream5,
    DMA2_Stream6,
    DMA2_Stream7,
};

const dma_map_obj dma_map[34] = {
    DMA_MAP(DMA_SPI_RX, 3, 1, 0, DMA_CHANNEL_0),
    //-------------------------------------------------
    DMA_MAP(DMA_SPI_RX, 3, 1, 2, DMA_CHANNEL_0),
    DMA_MAP(DMA_SPI_RX, 2, 1, 3, DMA_CHANNEL_0),
    DMA_MAP(DMA_SPI_TX, 2, 1, 4, DMA_CHANNEL_0),
    DMA_MAP(DMA_SPI_TX, 3, 1, 5, DMA_CHANNEL_0),
    //-------------------------------------------------
    DMA_MAP(DMA_SPI_TX, 3, 1, 7, DMA_CHANNEL_0),

    DMA_MAP(DMA_I2C_RX, 1, 1, 0, DMA_CHANNEL_1),
    //-------------------------------------------------
    DMA_MAP(DMA_TIM_UP, 7, 1, 2, DMA_CHANNEL_1),
    //-------------------------------------------------
    DMA_MAP(DMA_TIM_UP, 7, 1, 4, DMA_CHANNEL_1),
    DMA_MAP(DMA_I2C_RX, 1, 1, 5, DMA_CHANNEL_1),
    DMA_MAP(DMA_I2C_TX, 1, 1, 6, DMA_CHANNEL_1),
    DMA_MAP(DMA_I2C_TX, 1, 1, 7, DMA_CHANNEL_1),

    DMA_MAP(DMA_TIM_CH1,    4, 1, 0, DMA_CHANNEL_2),
    //-----------------------------------------------------
    DMA_MAP(DMA_I2S_EXT_RX, 3, 1, 2, DMA_CHANNEL_2),
    DMA_MAP(DMA_TIM_CH2,    4, 1, 3, DMA_CHANNEL_2),
    DMA_MAP(DMA_I2S_EXT_TX, 2, 1, 4, DMA_CHANNEL_2),
    DMA_MAP(DMA_I2S_EXT_TX, 3, 1, 5, DMA_CHANNEL_2),
    DMA_MAP(DMA_TIM_UP,     4, 1, 6, DMA_CHANNEL_2),
    DMA_MAP(DMA_TIM_CH3,    4, 1, 7, DMA_CHANNEL_2),

    DMA_MAP(DMA_I2S_EXT_RX, 3, 1, 0, DMA_CHANNEL_3),
    DMA_MAP(DMA_TIM_UP,     2, 1, 1, DMA_CHANNEL_3),
    DMA_MAP(DMA_TIM_CH3,    2, 1, 1, DMA_CHANNEL_3),
    DMA_MAP(DMA_I2C_RX,     3, 1, 2, DMA_CHANNEL_3),
    DMA_MAP(DMA_I2S_EXT_RX, 2, 1, 3, DMA_CHANNEL_3),
    DMA_MAP(DMA_I2C_TX,     3, 1, 4, DMA_CHANNEL_3),
    DMA_MAP(DMA_TIM_CH1,    2, 1, 5, DMA_CHANNEL_3),
    DMA_MAP(DMA_TIM_CH2,    2, 1, 6, DMA_CHANNEL_3),
    DMA_MAP(DMA_TIM_CH4,    2, 1, 6, DMA_CHANNEL_3),
    DMA_MAP(DMA_TIM_UP,     2, 1, 7, DMA_CHANNEL_3),
    DMA_MAP(DMA_TIM_CH4,    2, 1, 7, DMA_CHANNEL_3),

    //CH4
    //CH5
    //CH6
    //CH7

    //DMA2 -----

    //CH0
    //CH1
    //CH2

    //CH3
    DMA_MAP(DMA_SPI_RX, 1, 2, 0, DMA_CHANNEL_3),
    //-----------------2-------------------------------
    DMA_MAP(DMA_SPI_RX, 1, 2, 2, DMA_CHANNEL_3),
    DMA_MAP(DMA_SPI_TX, 1, 2, 3, DMA_CHANNEL_3),
    //-------------------------------------------------
    DMA_MAP(DMA_SPI_TX, 1, 2, 5, DMA_CHANNEL_3),
    //-------------------------------------------------
    //-------------------------------------------------

    //CH4
    //CH5
    //CH6
    //CH7
};
