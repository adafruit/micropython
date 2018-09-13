/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 hathach for Adafruit Industries
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

#ifndef FLASH_DEVICES_H_
#define FLASH_DEVICES_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
    uint32_t total_size;

    // Three response bytes to 0x90 JEDEC REMS ID command.
    uint8_t manufacturer_id;
    uint8_t device_id;

    // Status register value enable quad mode
    uint16_t status_quad_enable;

} qspi_flash_device_t;

// Settings for the Gigadevice GD25Q16C 2MiB SPI flash.
// Datasheet: http://www.gigadevice.com/wp-content/uploads/2017/12/DS-00086-GD25Q16C-Rev2.6.pdf
#define GD25Q16C {\
    .total_size = 2*1024*1024, \
    .manufacturer_id = 0xc8, \
    .device_id = 0x14, \
    .status_quad_enable = (1 << 9)\
}

#define MX25R6435F {\
    .total_size = 8*1024*1024, \
    .manufacturer_id = 0xc2, \
    .device_id = 0x17, \
    .status_quad_enable = (1 << 6)\
}

#ifdef __cplusplus
 }
#endif

#endif /* FLASH_DEVICES_H_ */
