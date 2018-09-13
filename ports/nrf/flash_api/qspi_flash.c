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

#include <string.h>

#include "mpconfigboard.h"
#include "nrfx_qspi.h"
#include "flash_api.h"
#include "qspi_flash.h"

#define NO_CACHE 0xffffffff

static uint32_t _cache_addr = NO_CACHE;
static uint8_t _cache_buf[FLASH_API_PAGE_SIZE] __attribute__((aligned(4)));

volatile static bool _qspi_complete = false;

void qspi_flash_isr (nrfx_qspi_evt_t event, void * p_context)
{
    (void) p_context;
    (void) event;

    _qspi_complete = true;
}

void qspi_flash_init (void) {

}

uint32_t qspi_flash_get_block_count (void) {
    return QSPI_FLASH_SIZE / FLASH_API_BLOCK_SIZE;
}

void qspi_flash_flush (void) {
    if ( _cache_addr == NO_CACHE ) return;

    if ( !(NRFX_SUCCESS == nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, _cache_addr)) ) return;
    while ( !_qspi_complete ) {
    }
    _qspi_complete = false;

    if ( !(NRFX_SUCCESS == nrfx_qspi_write(_cache_buf, FLASH_API_PAGE_SIZE, _cache_addr)) ) return;
    while ( !_qspi_complete ) {
    }
    _qspi_complete = false;

    _cache_addr = NO_CACHE;
}


mp_uint_t qspi_flash_write_blocks (const uint8_t *src, uint32_t lba, uint32_t count) {
    uint32_t dst = lba * FLASH_API_BLOCK_SIZE;
    uint32_t newAddr = dst & ~(FLASH_API_PAGE_SIZE - 1);

    if ( newAddr != _cache_addr ) {
        qspi_flash_flush();
        _cache_addr = newAddr;

        qspi_flash_read_blocks(_cache_buf, newAddr / FLASH_API_BLOCK_SIZE, FLASH_API_BLOCK_PER_PAGE);
    }

    memcpy(_cache_buf + (dst & (FLASH_API_PAGE_SIZE - 1)), src, count * FLASH_API_BLOCK_SIZE);
    return 0;
}

mp_uint_t qspi_flash_read_blocks (uint8_t* dst, uint32_t lba, uint32_t count) {
    nrfx_qspi_read(dst, count * FLASH_API_BLOCK_SIZE, lba * FLASH_API_BLOCK_SIZE);
    while ( !_qspi_complete ) {
    }
    _qspi_complete = false;

    return 0;
}


