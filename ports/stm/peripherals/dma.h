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

#ifndef __MICROPY_INCLUDED_STM32_PERIPHERALS_DMA_H__
#define __MICROPY_INCLUDED_STM32_PERIPHERALS_DMA_H__

#include STM32_HAL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DMA_SPI_RX,
    DMA_SPI_TX,
    DMA_I2C_RX,
    DMA_I2C_TX,
    DMA_I2S_EXT_RX,
    DMA_I2S_EXT_TX,
    DMA_TIM_UP,
    DMA_TIM_TRIG,
    DMA_TIM_CH1,
    DMA_TIM_CH2,
    DMA_TIM_CH3,
    DMA_TIM_CH4,
    DMA_UART_RX,
    DMA_UART_TX,
    DMA_USART_RX,
    DMA_USART_TX,
    DMA_SAI_A,
    DMA_SAI_B,
    DMA_DCMI,
    DMA_ADC,
    DMA_DAC,
    DMA_CRYP_IN,
    DMA_HASH_IN,
    DMA_CRYP_OUT,
    DMA_SDIO,
} dma_channel_type;

typedef struct {
    const dma_channel_type type;
    const uint8_t periph_index;
    const uint8_t dma_index;
    const uint8_t stream_index;
    const uint32_t channel;
} dma_map_obj;

#define DMA_MAP(t, i, d, s, c) \
{ \
    .type = t, \
    .periph_index = i, \
    .dma_index = d, \
    .stream_index = s, \
    .channel = c, \
}

void dma_reset(void);
bool stm32_peripherals_dma_init(DMA_HandleTypeDef* handle, uint8_t dma_index, uint8_t stream_idx,
                                    uint32_t channel, uint32_t direction, uint32_t priority, uint32_t subprio);
bool stm32_peripherals_dma_get_params(dma_channel_type dma_type, uint8_t periph_idx,
                                      uint8_t* dma_idx, uint8_t* stream_idx, uint32_t* channel);
extern const IRQn_Type dma1_irq_map[8];
extern const IRQn_Type dma2_irq_map[8];
extern const DMA_Stream_TypeDef* dma1_stream_map[8];
extern const DMA_Stream_TypeDef* dma2_stream_map[8];
extern const dma_map_obj dma_map[34];

#endif // __MICROPY_INCLUDED_STM32_PERIPHERALS_DMA_H__
