/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Lucian Copeland
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

#include "shared-bindings/spiperipheral/SPIPeripheral.h"
#include "common-hal/busio/SPI.h"

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "lib/utils/interrupt_char.h"

#include "shared-bindings/microcontroller/__init__.h"
#include "supervisor/shared/translate.h"
#include "shared-bindings/microcontroller/Pin.h"

#include "peripherals/dma.h"

// This module is largely quite similar to busio/SPI, and borrows the same reservation and
// pin location systems.

STATIC bool txrx_complete;
STATIC bool txrx_error;

STATIC DMA_HandleTypeDef *g_dma_handle_rx;
STATIC DMA_HandleTypeDef *g_dma_handle_tx;

spiperipheral_spi_peripheral_obj_t * handles[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

STATIC void spi_assign_irq(spiperipheral_spi_peripheral_obj_t *self, SPI_TypeDef * SPIx);

void spiperipheral_reset(void) {
    if (g_dma_handle_tx) {
        HAL_DMA_DeInit(g_dma_handle_tx);
    }
    if (g_dma_handle_rx) {
        HAL_DMA_DeInit(g_dma_handle_rx);
    }
    HAL_NVIC_DisableIRQ(DMA1_Stream4_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Stream3_IRQn);
    HAL_NVIC_DisableIRQ(SPI2_IRQn);

    __HAL_RCC_DMA1_CLK_DISABLE();
    __HAL_RCC_DMA2_CLK_DISABLE();
    for (int i = 0; i < 6; i++) {
        g_dma_handle_rx = NULL;
        g_dma_handle_tx = NULL;
        handles[i] = NULL;
    }
}

void common_hal_spiperipheral_spi_peripheral_construct(spiperipheral_spi_peripheral_obj_t *self,
         const mcu_pin_obj_t * sck, const mcu_pin_obj_t * mosi,
         const mcu_pin_obj_t * miso, const mcu_pin_obj_t * cs) {

    // CS can be any pin and does not need to be checked against the others.
    int periph_index = spi_check_pins((busio_spi_obj_t*)self, sck, mosi, miso);
    self->cs_pin = cs;

    SPI_TypeDef * SPIx = mcu_spi_banks[periph_index - 1];

    //Start GPIO for each pin
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin_mask(sck->number);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = self->sck->altfn_index;
    HAL_GPIO_Init(pin_port(sck->port), &GPIO_InitStruct);

    GPIO_InitStruct.Pin = pin_mask(cs->number);
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(pin_port(sck->port), &GPIO_InitStruct);

    if (self->mosi != NULL) {
        GPIO_InitStruct.Pin = pin_mask(mosi->number);
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = self->mosi->altfn_index;
        HAL_GPIO_Init(pin_port(mosi->port), &GPIO_InitStruct);
    }

    if (self->miso != NULL) {
        GPIO_InitStruct.Pin = pin_mask(miso->number);
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = self->miso->altfn_index;
        HAL_GPIO_Init(pin_port(miso->port), &GPIO_InitStruct);
    }

    spi_clock_enable(1 << (self->sck->periph_index - 1));
    reserved_spi[self->sck->periph_index - 1] = true;
    spi_assign_irq(self, SPIx);
    handles[self->sck->periph_index - 1] = self;

    self->handle.Instance = SPIx;
    self->handle.Init.Mode = SPI_MODE_SLAVE;
    // Direction change only required for RX-only, see RefMan RM0090:884
    self->handle.Init.Direction = (self->mosi == NULL) ? SPI_DIRECTION_2LINES_RXONLY : SPI_DIRECTION_2LINES;
    self->handle.Init.DataSize = SPI_DATASIZE_8BIT;
    self->handle.Init.CLKPolarity = SPI_POLARITY_LOW;
    self->handle.Init.CLKPhase = SPI_PHASE_1EDGE;
    self->handle.Init.NSS = SPI_NSS_SOFT;
    self->handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    self->handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    self->handle.Init.TIMode = SPI_TIMODE_DISABLE;
    self->handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    self->handle.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&self->handle) != HAL_OK)
    {
        mp_raise_ValueError(translate("SPI Init Error"));
    }
    self->polarity = 0;
    self->phase = 0;
    self->bits = 8;

    common_hal_mcu_pin_claim(sck);
    common_hal_mcu_pin_claim(cs);
    if (self->mosi != NULL) {
        common_hal_mcu_pin_claim(mosi);
    }
    if (self->miso != NULL) {
        common_hal_mcu_pin_claim(miso);
    }

    // ---------------------------
    // DMA Peripheral module setup (not working)
    // ---------------------------

    // uint8_t dma_idx = 0;
    // uint8_t dma_stream_idx = 0;
    // uint32_t dma_channel = 0;

    // // TX
    // stm32_peripherals_dma_get_params(DMA_SPI_TX, periph_index, &dma_idx, &dma_stream_idx, &dma_channel);
    // stm32_peripherals_dma_init(&self->dma_handle_tx, dma_idx, dma_stream_idx, dma_channel,
    //                             DMA_MEMORY_TO_PERIPH, DMA_PRIORITY_LOW, 1);
    // __HAL_LINKDMA(&self->handle, hdmatx, self->dma_handle_tx);

    // // RX
    // stm32_peripherals_dma_get_params(DMA_SPI_RX, periph_index, &dma_idx, &dma_stream_idx, &dma_channel);
    // stm32_peripherals_dma_init(&self->dma_handle_rx, dma_idx, dma_stream_idx, dma_channel,
    //                             DMA_PERIPH_TO_MEMORY, DMA_PRIORITY_HIGH, 0);
    // __HAL_LINKDMA(&self->handle, hdmarx, self->dma_handle_rx);

    // HAL_NVIC_SetPriority(SPI2_IRQn, 0, 2);
    // HAL_NVIC_EnableIRQ(SPI2_IRQn);


    // ---------------------------
    // Manual DMA setup
    // ---------------------------

    __HAL_RCC_DMA1_CLK_ENABLE();

    // TX
    self->dma_handle_tx.Instance                 = DMA1_Stream4;
    self->dma_handle_tx.Init.Channel             = DMA_CHANNEL_0;
    self->dma_handle_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    self->dma_handle_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    self->dma_handle_tx.Init.MemInc              = DMA_MINC_ENABLE;
    self->dma_handle_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    self->dma_handle_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    self->dma_handle_tx.Init.Mode                = DMA_NORMAL;
    self->dma_handle_tx.Init.Priority            = DMA_PRIORITY_LOW;
    self->dma_handle_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    self->dma_handle_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    self->dma_handle_tx.Init.MemBurst            = DMA_MBURST_INC4;
    self->dma_handle_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
    HAL_DMA_Init(&self->dma_handle_tx);
    __HAL_LINKDMA(&self->handle, hdmatx, self->dma_handle_tx);

    //register global handle for interrupt
    g_dma_handle_tx = &self->dma_handle_tx;

    // RX
    self->dma_handle_rx.Instance                 = DMA1_Stream3;
    self->dma_handle_rx.Init.Channel             = DMA_CHANNEL_0;
    self->dma_handle_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    self->dma_handle_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    self->dma_handle_rx.Init.MemInc              = DMA_MINC_ENABLE;
    self->dma_handle_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    self->dma_handle_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    self->dma_handle_rx.Init.Mode                = DMA_NORMAL;
    self->dma_handle_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    self->dma_handle_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    self->dma_handle_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    self->dma_handle_rx.Init.MemBurst            = DMA_MBURST_INC4;
    self->dma_handle_rx.Init.PeriphBurst         = DMA_PBURST_INC4;
    HAL_DMA_Init(&self->dma_handle_rx);
    __HAL_LINKDMA(&self->handle, hdmarx, self->dma_handle_rx);

    //register global handle for interrupt
    g_dma_handle_rx = &self->dma_handle_rx;

    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
    HAL_NVIC_SetPriority(SPI2_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);

    // ---------------------------
    // End of DMA
    // ---------------------------
}


bool common_hal_spiperipheral_spi_peripheral_deinited(spiperipheral_spi_peripheral_obj_t *self) {
    return self->sck->pin == NULL;
}

void common_hal_spiperipheral_spi_peripheral_deinit(spiperipheral_spi_peripheral_obj_t *self) {
    if (common_hal_spiperipheral_spi_peripheral_deinited(self)) {
        return;
    }
    spi_clock_disable(1<<(self->sck->periph_index - 1));
    reserved_spi[self->sck->periph_index - 1] = false;
    never_reset_spi[self->sck->periph_index - 1] = false;
    handles[self->sck->periph_index - 1] = NULL;

    reset_pin_number(self->sck->pin->port,self->sck->pin->number);
    if (self->mosi != NULL) {
        reset_pin_number(self->mosi->pin->port,self->mosi->pin->number);
    }
    if (self->miso != NULL) {
        reset_pin_number(self->miso->pin->port,self->miso->pin->number);
    }
    self->sck = NULL;
    self->mosi = NULL;
    self->miso = NULL;
}

bool common_hal_spiperipheral_spi_peripheral_spi_ready(spiperipheral_spi_peripheral_obj_t *self) {
    return HAL_SPI_GetState(&self->handle) == HAL_SPI_STATE_READY;
}

bool common_hal_spiperipheral_spi_peripheral_transaction_complete(spiperipheral_spi_peripheral_obj_t *self) {
    return txrx_complete;
}

bool common_hal_spiperipheral_spi_peripheral_transaction_error(spiperipheral_spi_peripheral_obj_t *self) {
    return txrx_error;
}

STATIC uint8_t incr_counter = 0;
STATIC uint8_t redata_out[72];
STATIC uint8_t redata_in[72];

bool common_hal_spiperipheral_spi_peripheral_wait_for_transaction(spiperipheral_spi_peripheral_obj_t *self,
        const uint8_t *data_out, uint8_t *data_in, size_t len) {
    // if (!common_hal_spiperipheral_spi_peripheral_spi_ready(self)) {
    //     txrx_complete = false;
    //     mp_raise_RuntimeError(translate("SPI transaction not complete"));
    // }
    // if (common_hal_spiperipheral_spi_peripheral_transaction_error(self)) {
    //     txrx_error = false;
    //     mp_raise_RuntimeError(translate("Error in last transaction"));
    // }

    // txrx_complete = false;
    // txrx_error = false;
    __disable_irq();
    HAL_SPI_DMAStop(&self->handle);

    len = 72;
    incr_counter++; // = redata_in[1] + 1;
    redata_out[0] = 0x01;
    redata_out[1] = incr_counter;
    redata_out[2] = 0x03;

    // Wait until the CS pin has dropped before attempting a transaction.
    // while(HAL_GPIO_ReadPin(pin_port(self->cs_pin->port), pin_mask(self->cs_pin->number))) {
    //     RUN_BACKGROUND_TASKS;
    //     // Allow user to break out of a timeout with a KeyboardInterrupt.
    //     if ( mp_hal_is_interrupted() ) {
    //         return 0;
    //     }
    // }

    return HAL_SPI_TransmitReceive_DMA(&self->handle, (uint8_t*)redata_out, (uint8_t *)redata_in, (uint16_t)len) == HAL_OK;
    __enable_irq();
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    txrx_complete = true;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    txrx_error = true;
}

void DMA1_Stream3_IRQHandler(void)
{
    if (g_dma_handle_rx) {
        HAL_DMA_IRQHandler(g_dma_handle_rx);
    }
}

void DMA1_Stream4_IRQHandler(void)
{
    if (g_dma_handle_tx) {
        HAL_DMA_IRQHandler(g_dma_handle_tx);
    }
}

// void SPI2_IRQHandler(void)
// {
//     HAL_SPI_IRQHandler(&SpiHandle);
// }

void SPI1_IRQHandler(void) {
    if (handles[0]) {
        HAL_SPI_IRQHandler(&handles[0]->handle);
    }
}

void SPI2_IRQHandler(void) {
    if (handles[1]) {
        HAL_SPI_IRQHandler(&handles[1]->handle);
    }
}

void SPI3_IRQHandler(void) {
    if (handles[2]) {
        HAL_SPI_IRQHandler(&handles[2]->handle);
    }
}

void SPI4_IRQHandler(void) {
    if (handles[3]) {
        HAL_SPI_IRQHandler(&handles[3]->handle);
    }
}

void SPI5_IRQHandler(void) {
    if (handles[4]) {
        HAL_SPI_IRQHandler(&handles[4]->handle);
    }
}

void SPI6_IRQHandler(void) {
    if (handles[5]) {
        HAL_SPI_IRQHandler(&handles[5]->handle);
    }
}

STATIC void spi_assign_irq(spiperipheral_spi_peripheral_obj_t *self, SPI_TypeDef * SPIx) {
    #ifdef SPI1
    if (SPIx == SPI1) {
        self->irq = SPI1_IRQn;
    }
    #endif
    #ifdef SPI2
    if (SPIx == SPI2) {
        self->irq = SPI2_IRQn;
    }
    #endif
    #ifdef SPI3
    if (SPIx == SPI3) {
        self->irq = SPI3_IRQn;
    }
    #endif
    #ifdef SPI4
    if (SPIx == SPI4) {
        self->irq = SPI4_IRQn;
    }
    #endif
    #ifdef SPI5
    if (SPIx == SPI5) {
        self->irq = SPI5_IRQn;
    }
    #endif
    #ifdef SPI6
    if (SPIx == SPI6) {
        self->irq = SPI6_IRQn;
    }
    #endif
}
