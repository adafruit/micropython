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

#include "ili9341.h"

inline void spi_write(ili9341_obj_t *self, const void * data,
        size_t size) {
    common_hal_digitalio_digitalinout_set_value(self->cs, false);

    common_hal_busio_spi_write(self->spi, data, size);

    common_hal_digitalio_digitalinout_set_value(self->cs, true);
}

inline void write_command(ili9341_obj_t *self, uint8_t c) {
    common_hal_digitalio_digitalinout_set_value(self->dc, false);

    spi_write(self, &c, sizeof(c));
}

inline void write_data(ili9341_obj_t *self, uint8_t c) {
    common_hal_digitalio_digitalinout_set_value(self->dc, true);

    spi_write(self, &c, sizeof(c));
}

void display_init9341_write_init(ili9341_obj_t *self) {
    write_command(self, ILI9341_SWRESET);
    mp_hal_delay_ms(120);

    write_command(self, ILI9341_DISPOFF);
    mp_hal_delay_ms(120);

    write_command(self, ILI9341_PWCTRB);
    write_data(self, 0x00);
    write_data(self, 0XC1);
    write_data(self, 0X30);

    write_command(self, ILI9341_TIMCTRA);
    write_data(self, 0x85);
    write_data(self, 0x00);
    write_data(self, 0x78);

    write_command(self, ILI9341_PWCTRSEQ);
    write_data(self, 0x39);
    write_data(self, 0x2C);
    write_data(self, 0x00);
    write_data(self, 0x34);
    write_data(self, 0x02);

    write_command(self, ILI9341_PUMP);
    write_data(self, 0x20);

    write_command(self, ILI9341_TIMCTRB);
    write_data(self, 0x00);
    write_data(self, 0x00);

    write_command(self, ILI9341_PWCTR1);
    write_data(self, 0x23);

    write_command(self, ILI9341_PWCTR2);
    write_data(self, 0x10);

    write_command(self, ILI9341_VMCTR1);
    write_data(self, 0x3e);
    write_data(self, 0x28);

    write_command(self, ILI9341_VMCTR2);
    write_data(self, 0x86);

    write_command(self, ILI9341_MADCTL);
    write_data(self, 0x48);

    write_command(self, ILI9341_PIXFMT);
    write_data(self, 0x55);

    write_command(self, ILI9341_FRMCTR1);
    write_data(self, 0x00);
    write_data(self, 0x18);

    write_command(self, ILI9341_DFUNCTR);
    write_data(self, 0x08);
    write_data(self, 0x82);
    write_data(self, 0x27);

    write_command(self, ILI9341_ENGMCTR);
    write_data(self, 0x00);

    write_command(self, ILI9341_GAMMASET);
    write_data(self, 0x01);

    write_command(self, ILI9341_GMCTRP1);
    write_data(self, 0x0F);
    write_data(self, 0x31);
    write_data(self, 0x2B);
    write_data(self, 0x0C);
    write_data(self, 0x0E);
    write_data(self, 0x08);
    write_data(self, 0x4E);
    write_data(self, 0xF1);
    write_data(self, 0x37);
    write_data(self, 0x07);
    write_data(self, 0x10);
    write_data(self, 0x03);
    write_data(self, 0x0E);
    write_data(self, 0x09);
    write_data(self, 0x00);

    write_command(self, ILI9341_GMCTRN1);
    write_data(self, 0x00);
    write_data(self, 0x0E);
    write_data(self, 0x14);
    write_data(self, 0x03);
    write_data(self, 0x11);
    write_data(self, 0x07);
    write_data(self, 0x31);
    write_data(self, 0xC1);
    write_data(self, 0x48);
    write_data(self, 0x08);
    write_data(self, 0x0F);
    write_data(self, 0x0C);
    write_data(self, 0x31);
    write_data(self, 0x36);
    write_data(self, 0x0F);

    write_command(self, ILI9341_SLPOUT);
    mp_hal_delay_ms(120);

    write_command(self, ILI9341_DISPON);
}

void display_ili9341_set_rotation(ili9341_obj_t *self,
        display_rotation_t rotation)
{
    uint8_t data;

    self->rotation = rotation;

    write_command(self, ILI9341_MADCTL);

    switch (self->rotation) {
        case ROTATION_90:  data = (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR); break;
        case ROTATION_180: data = (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR); break;
        case ROTATION_270: data = (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR); break;
        default:           data = (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR); break;
    }

    write_data(self, data);
}

void display_ili9341_set_addr_window(ili9341_obj_t *self, uint16_t x_0,
        uint16_t y_0, uint16_t x_1, uint16_t y_1) {
    write_command(self, ILI9341_CASET);
    write_data(self, x_0 >> 8);
    write_data(self, x_0);
    write_data(self, x_1 >> 8);
    write_data(self, x_1);

    write_command(self, ILI9341_PASET);
    write_data(self, y_0 >> 8);
    write_data(self, y_0);
    write_data(self, y_1 >> 8);
    write_data(self, y_1);

    write_command(self, ILI9341_RAMWR);
}

void display_ili9341_pixel(ili9341_obj_t *self, uint16_t x, uint16_t y,
        uint32_t color) {
    const uint8_t data[2] = {color >> 8, color};

    display_ili9341_set_addr_window(self, x, y, x, y);

    common_hal_digitalio_digitalinout_set_value(self->dc, true);

    spi_write(self, data, sizeof(data));

    common_hal_digitalio_digitalinout_set_value(self->dc, false);
}

void display_ili9341_rect(ili9341_obj_t *self, uint16_t x, uint16_t y,
        uint16_t width, uint16_t height, uint32_t color)
{
    const uint8_t data[2] = { color >> 8, color };

    display_ili9341_set_addr_window(self, x, y, x + width - 1, y + height - 1);

    common_hal_digitalio_digitalinout_set_value(self->dc, true);

    // Duff's device algorithm
    uint32_t i = (height * width + 7) / 8;

    switch ((height * width) % 8) {
        case 0: do { spi_write(self, data, sizeof(data));
        case 7:      spi_write(self, data, sizeof(data));
        case 6:      spi_write(self, data, sizeof(data));
        case 5:      spi_write(self, data, sizeof(data));
        case 4:      spi_write(self, data, sizeof(data));
        case 3:      spi_write(self, data, sizeof(data));
        case 2:      spi_write(self, data, sizeof(data));
        case 1:      spi_write(self, data, sizeof(data));
                } while (--i > 0);
    }

    common_hal_digitalio_digitalinout_set_value(self->dc, false);
}
