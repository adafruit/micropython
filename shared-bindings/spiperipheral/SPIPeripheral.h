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

#ifndef MICROPY_INCLUDED_SHARED_BINDINGS_SPIPERIPHERAL_SPI_PERIPHERAL_H
#define MICROPY_INCLUDED_SHARED_BINDINGS_SPIPERIPHERAL_SPI_PERIPHERAL_H

#include "py/obj.h"

#include "common-hal/microcontroller/Pin.h"
#include "common-hal/spiperipheral/SPIPeripheral.h"

// Type object used in Python. Should be shared between ports.
extern const mp_obj_type_t spiperipheral_spi_peripheral_type;

// Construct an underlying SPI object.
void common_hal_spiperipheral_spi_peripheral_construct(spiperipheral_spi_peripheral_obj_t *self,
         const mcu_pin_obj_t * sck, const mcu_pin_obj_t * mosi,
         const mcu_pin_obj_t * miso, const mcu_pin_obj_t * cs);

bool common_hal_spiperipheral_spi_peripheral_deinited(spiperipheral_spi_peripheral_obj_t *self);
void common_hal_spiperipheral_spi_peripheral_deinit(spiperipheral_spi_peripheral_obj_t *self);

bool common_hal_spiperipheral_spi_peripheral_spi_ready(spiperipheral_spi_peripheral_obj_t *self);
bool common_hal_spiperipheral_spi_peripheral_transaction_complete(spiperipheral_spi_peripheral_obj_t *self);
bool common_hal_spiperipheral_spi_peripheral_transaction_error(spiperipheral_spi_peripheral_obj_t *self);

bool common_hal_spiperipheral_spi_peripheral_wait_for_transaction(spiperipheral_spi_peripheral_obj_t *self,
        const uint8_t *data_out, uint8_t *data_in, size_t len);

#endif // MICROPY_INCLUDED_SHARED_BINDINGS_SPIPERIPHERAL_SPI_PERIPHERAL_H
