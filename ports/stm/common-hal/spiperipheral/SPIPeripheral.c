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

// This module is largely quite similar to busio/SPI, and borrows the same reservation and
// pin location systems.

STATIC bool txrx_complete;
STATIC bool txrx_error;

spiperipheral_spi_peripheral_obj_t * handles[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

STATIC void spi_assign_irq(spiperipheral_spi_peripheral_obj_t *self, SPI_TypeDef * SPIx);

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
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
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

    HAL_NVIC_SetPriority(self->irq, 0, 1);
    HAL_NVIC_EnableIRQ(self->irq);
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

bool common_hal_spiperipheral_spi_peripheral_wait_for_transaction(spiperipheral_spi_peripheral_obj_t *self,
        const uint8_t *data_out, uint8_t *data_in, size_t len) {
    if (!common_hal_spiperipheral_spi_peripheral_spi_ready(self)) {
        mp_raise_RuntimeError(translate("SPI transaction not complete"));
    }
    if (common_hal_spiperipheral_spi_peripheral_transaction_error(self)) {
        mp_raise_RuntimeError(translate("Error in last transaction"));
    }

    // Wait until the CS pin has dropped before attempting a transaction.
    while(HAL_GPIO_ReadPin(pin_port(self->cs_pin->port), pin_mask(self->cs_pin->number))) {
        RUN_BACKGROUND_TASKS;
        // Allow user to break out of a timeout with a KeyboardInterrupt.
        if ( mp_hal_is_interrupted() ) {
            return 0;
        }
    }

    txrx_complete = false;
    txrx_error = false;

    // Is there a compelling reason to use HAL_SPI_TransmitReceive_DMA here instead?
    return HAL_SPI_TransmitReceive_IT(&self->handle, (uint8_t*)data_out, (uint8_t *)data_in, (uint16_t)len) == HAL_OK;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    txrx_complete = true;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    txrx_error = true;
}

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
