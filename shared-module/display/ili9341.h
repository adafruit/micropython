/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Artur Pacholec
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

#ifndef MICROPY_INCLUDED_SHARED_MODULE_DISPLAY_ILI9341_H
#define MICROPY_INCLUDED_SHARED_MODULE_DISPLAY_ILI9341_H

#include "py/obj.h"
#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/display/rotation.h"

#define ILI9341_NOP         0x00
#define ILI9341_SWRESET     0x01
#define ILI9341_RDDID       0x04
#define ILI9341_RDDST       0x09

#define ILI9341_SLPIN       0x10
#define ILI9341_SLPOUT      0x11
#define ILI9341_PTLON       0x12
#define ILI9341_NORON       0x13

#define ILI9341_RDMODE      0x0A
#define ILI9341_RDMADCTL    0x0B
#define ILI9341_RDPIXFMT    0x0C
#define ILI9341_RDIMGFMT    0x0D
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF      0x20
#define ILI9341_INVON       0x21
#define ILI9341_GAMMASET    0x26
#define ILI9341_DISPOFF     0x28
#define ILI9341_DISPON      0x29

#define ILI9341_CASET       0x2A
#define ILI9341_PASET       0x2B
#define ILI9341_RAMWR       0x2C
#define ILI9341_RAMRD       0x2E

#define ILI9341_PTLAR       0x30
#define ILI9341_MADCTL      0x36
#define ILI9341_PIXFMT      0x3A

#define ILI9341_FRMCTR1     0xB1
#define ILI9341_FRMCTR2     0xB2
#define ILI9341_FRMCTR3     0xB3
#define ILI9341_INVCTR      0xB4
#define ILI9341_DFUNCTR     0xB6

#define ILI9341_PWCTR1      0xC0
#define ILI9341_PWCTR2      0xC1
#define ILI9341_PWCTR3      0xC2
#define ILI9341_PWCTR4      0xC3
#define ILI9341_PWCTR5      0xC4
#define ILI9341_VMCTR1      0xC5
#define ILI9341_VMCTR2      0xC7
#define ILI9341_PWCTRSEQ    0xCB
#define ILI9341_PWCTRA      0xCD
#define ILI9341_PWCTRB      0xCF

#define ILI9341_RDID1       0xDA
#define ILI9341_RDID2       0xDB
#define ILI9341_RDID3       0xDC
#define ILI9341_RDID4       0xDD

#define ILI9341_GMCTRP1     0xE0
#define ILI9341_GMCTRN1     0xE1
#define ILI9341_DGMCTR1     0xE2
#define ILI9341_DGMCTR2     0xE3
#define ILI9341_TIMCTRA     0xE8
#define ILI9341_TIMCTRB     0xEA

#define ILI9341_ENGMCTR     0xF2
#define ILI9341_INCTR       0xF6
#define ILI9341_PUMP        0xF7

#define ILI9341_MADCTL_MY   0x80
#define ILI9341_MADCTL_MX   0x40
#define ILI9341_MADCTL_MV   0x20
#define ILI9341_MADCTL_ML   0x10
#define ILI9341_MADCTL_RGB  0x00
#define ILI9341_MADCTL_BGR  0x08
#define ILI9341_MADCTL_MH   0x04

typedef struct {
    mp_obj_base_t base;
    display_rotation_t rotation;
    busio_spi_obj_t *spi;
    digitalio_digitalinout_obj_t *cs;
    digitalio_digitalinout_obj_t *dc;
} ili9341_obj_t;

void display_init9341_write_init(ili9341_obj_t *self);
void display_ili9341_set_rotation(ili9341_obj_t *self,
        display_rotation_t rotation);
void display_ili9341_set_addr_window(ili9341_obj_t *self, uint16_t x_0,
        uint16_t y_0, uint16_t x_1, uint16_t y_1);
void display_ili9341_pixel(ili9341_obj_t *self, uint16_t x, uint16_t y,
        uint32_t color);
void display_ili9341_rect(ili9341_obj_t *self, uint16_t x, uint16_t y,
        uint16_t width, uint16_t height, uint32_t color);

#endif  // MICROPY_INCLUDED_SHARED_MODULE_DISPLAY_ILI9341_H
