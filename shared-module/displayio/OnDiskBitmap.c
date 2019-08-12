/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Scott Shawcroft for Adafruit Industries
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

#include "shared-bindings/displayio/OnDiskBitmap.h"

#include <string.h>

#include "py/mperrno.h"
#include "py/runtime.h"

static uint32_t read_word(uint16_t* bmp_header, uint16_t index) {
    return bmp_header[index] | bmp_header[index + 1] << 16;
}

void common_hal_displayio_ondiskbitmap_construct(displayio_ondiskbitmap_t *self, pyb_file_obj_t* file) {
    // Load the wave
    self->file = file;
    uint16_t bmp_header[69];
    f_rewind(&self->file->fp);
    UINT bytes_read;
    if (f_read(&self->file->fp, bmp_header, 138, &bytes_read) != FR_OK) {
        mp_raise_OSError(MP_EIO);
    }
    if (bytes_read != 138 ||
        memcmp(bmp_header, "BM", 2) != 0) {
        mp_raise_ValueError(translate("Invalid BMP file"));
    }

    // We can't cast because we're not aligned.
    self->data_offset = read_word(bmp_header, 5);

    uint32_t header_size = read_word(bmp_header, 7);
    uint16_t bits_per_pixel = bmp_header[14];
    uint32_t compression = read_word(bmp_header, 15);
    uint32_t number_of_colors = read_word(bmp_header, 23);

    bool indexed = ((bits_per_pixel <= 8) && (number_of_colors != 0));
    self->bitfield_compressed  = (compression == 3);
    self->bits_per_pixel = bits_per_pixel;
    self->width = read_word(bmp_header, 9);
    self->height = read_word(bmp_header, 11);

    if (bits_per_pixel == 16){
        if (((header_size >= 56)) || (self->bitfield_compressed)) {
            self->r_bitmask = read_word(bmp_header, 27);
            self->g_bitmask = read_word(bmp_header, 29);
            self->b_bitmask = read_word(bmp_header, 31);

        } else { // no compression or short header means 5:5:5
            self->r_bitmask = 0x7c00;
            self->g_bitmask = 0x3e0;
            self->b_bitmask = 0x1f;
        }
    } else if ((indexed) && (self->bits_per_pixel != 1)) {
        uint16_t palette_size = number_of_colors * sizeof(uint32_t);
        uint16_t palette_offset = 0xe + header_size;

        self->palette_data = m_malloc(palette_size, false);

        f_rewind(&self->file->fp);
        f_lseek(&self->file->fp, palette_offset);

        UINT palette_bytes_read;
        if (f_read(&self->file->fp, self->palette_data, palette_size, &palette_bytes_read) != FR_OK) {
            mp_raise_OSError(MP_EIO);
        }
        if (palette_bytes_read != palette_size) {
            mp_raise_ValueError(translate("Unable to read color palette data"));
        }


    } else if (!(header_size == 12 || header_size == 40 || header_size == 108 || header_size == 124)) {
        mp_raise_ValueError_varg(translate("Only Windows format, uncompressed BMP supported: given header size is %d"), header_size);
    }

    if ((bits_per_pixel == 4 ) || (( bits_per_pixel == 8) && (number_of_colors == 0))) {
        mp_raise_ValueError_varg(translate("Only monochrome, indexed 8bpp, and 16bpp or greater BMPs supported: %d bpp given"), bits_per_pixel);
    }

    if (self->bits_per_pixel >=8){
        self->stride = (self->width * (bits_per_pixel / 8));
        // Rows are word aligned.
        if (self->stride % 4 != 0) {
            self->stride += 4 - self->stride % 4;
        }

    } else {
        uint32_t bit_stride = self->width;
        if (bit_stride % 32 != 0) {
            bit_stride += 32 - bit_stride % 32;
        }
        self->stride = (bit_stride / 8);
    }

    #if CIRCUITPY_DISPLAYIO_DITHER
        // Dither defaults for 565 display
        //self->dither_mask_r = 7;
        //self->dither_mask_g = 3;
        //self->dither_mask_b = 7;

        // Dither Off
        self->dither_mask_r = 0;
        self->dither_mask_g = 0;
        self->dither_mask_b = 0;
    #endif
}

#if CIRCUITPY_DISPLAYIO_DITHER
// Perlin Noise code use to generate dithering from:
// https://gist.github.com/nowl/828013
static uint8_t hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                      185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                      9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                      70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                      203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                      164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                      228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                      232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                      193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                      101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                      135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                      114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

static int noise2(int x, int y)
{
     int tmp = hash[(y) % 256];
     return hash[(tmp + x) % 256];
}

#endif

uint32_t common_hal_displayio_ondiskbitmap_get_pixel(displayio_ondiskbitmap_t *self,
        int16_t x, int16_t y) {
    if (x < 0 || x >= self->width || y < 0 || y >= self->height) {
        return 0;
    }

    uint32_t location;
    uint8_t bytes_per_pixel = (self->bits_per_pixel / 8)  ? (self->bits_per_pixel /8) : 1;
    if (self->bits_per_pixel >= 8){
        location = self->data_offset + (self->height - y - 1) * self->stride + x * bytes_per_pixel;
    } else {
        location = self->data_offset + (self->height - y - 1) * self->stride + x / 8;
    }
    // We don't cache here because the underlying FS caches sectors.
    f_lseek(&self->file->fp, location);

    UINT bytes_read;
    uint32_t pixel_data = 0;
    uint32_t result = f_read(&self->file->fp, &pixel_data, bytes_per_pixel, &bytes_read);
    if (result == FR_OK) {
        uint32_t tmp = 0;
        uint8_t red;
        uint8_t green;
        uint8_t blue;

        #if CIRCUITPY_DISPLAYIO_DITHER
        if (self->dither_mask_r > 0 || self->dither_mask_g > 0 || self->dither_mask_b > 0 ) {
            uint8_t randr  = (noise2(x,y) * 255) & self->dither_mask_r;
            uint8_t randg  = (noise2(x+33,y) * 255) & self->dither_mask_g;
            uint8_t randb  = (noise2(x,y+33) * 255) & self->dither_mask_b;

            if (self->bits_per_pixel == 1) {
                uint8_t bit_offset = x%8;
                tmp = ( pixel_data & (0x80 >> (bit_offset))) >> (7 - bit_offset);
                if (tmp == 1) {
                    return 0x00FFFFFF;
                } else {
                    return 0x00000000;
                }
            } else if (bytes_per_pixel == 1) {
                blue = MIN(255,((self->palette_data[pixel_data] & 0xFF) >> 0) + randb);
                red = MIN(255,((self->palette_data[pixel_data] & 0xFF0000) >> 16) + randr);
                green = MIN(255,((self->palette_data[pixel_data] & 0xFF00) >> 8) + randg);
                tmp = (red << 16 | green << 8 | blue );
                return tmp;
            } else if (bytes_per_pixel == 2) {
                // No Dithering done because assuming it as already been done
                if (self->g_bitmask == 0x07e0) { // 565
                    red =((pixel_data & self->r_bitmask) >>11);
                    green = ((pixel_data & self->g_bitmask) >>5);
                    blue = ((pixel_data & self->b_bitmask) >> 0);
                } else { // 555
                    red =((pixel_data & self->r_bitmask) >>10);
                    green = ((pixel_data & self->g_bitmask) >>4);
                    blue = ((pixel_data & self->b_bitmask) >> 0);
                }
                tmp = (red << 19 | green << 10 | blue << 3);
                return tmp;
            } else  if ((bytes_per_pixel == 4) && (self->bitfield_compressed)) {
                blue = MIN(255, (pixel_data & 0xFF) + randb);
                green = MIN(255, ((pixel_data >> 8) & 0xFF) + randg);
                red = MIN(255,((pixel_data >> 16) & 0xFF)  + randr);

                return (red << 16 | green << 8 | blue );
            } else {
                blue = MIN(255, (pixel_data & 0xFF) + randb);
                green = MIN(255, ((pixel_data >> 8) & 0xFF) + randg);
                red = MIN(255,((pixel_data >> 16) & 0xFF)  + randr);

                return (red << 16 | green << 8 | blue );
            }
        } 
        #endif // CIRCUITPY_DISPLAYIO_DITHER
        
        if (self->bits_per_pixel == 1) {
            uint8_t bit_offset = x%8;
            tmp = ( pixel_data & (0x80 >> (bit_offset))) >> (7 - bit_offset);
            if (tmp == 1) {
                return 0x00FFFFFF;
            } else {
                return 0x00000000;
            }
        } else if (bytes_per_pixel == 1) {
            blue = ((self->palette_data[pixel_data] & 0xFF) >> 0);
            red = ((self->palette_data[pixel_data] & 0xFF0000) >> 16);
            green = ((self->palette_data[pixel_data] & 0xFF00) >> 8);
            tmp = (red << 16 | green << 8 | blue );
            return tmp;
        } else if (bytes_per_pixel == 2) {
            if (self->g_bitmask == 0x07e0) { // 565
                red =((pixel_data & self->r_bitmask) >>11);
                green = ((pixel_data & self->g_bitmask) >>5);
                blue = ((pixel_data & self->b_bitmask) >> 0);
            } else { // 555
                red =((pixel_data & self->r_bitmask) >>10);
                green = ((pixel_data & self->g_bitmask) >>4);
                blue = ((pixel_data & self->b_bitmask) >> 0);
            }
            tmp = (red << 19 | green << 10 | blue << 3);
            return tmp;
        } else if ((bytes_per_pixel == 4) && (self->bitfield_compressed)) {
            return pixel_data & 0x00FFFFFF;
        } else {
            return pixel_data;
        }
    }

    return 0;
}

uint16_t common_hal_displayio_ondiskbitmap_get_height(displayio_ondiskbitmap_t *self) {
    return self->height;
}

uint16_t common_hal_displayio_ondiskbitmap_get_width(displayio_ondiskbitmap_t *self) {
    return self->width;
}
