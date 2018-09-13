/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include <stdint.h>
#include <string.h>

#include "flash_api/flash_api.h"
#include "lib/oofatfs/ff.h"
#include "supervisor/shared/rgb_led_status.h"

#include "nrf_nvmc.h"

#ifdef BLUETOOTH_SD
#include "nrf_sdm.h"
#endif

// defined in linker
extern uint32_t __fatfs_flash_start_addr[];
extern uint32_t __fatfs_flash_length[];

static inline uint32_t log2phy_addr (uint32_t addr) {
    return ((uint32_t) __fatfs_flash_start_addr) + addr;
}

void internal_flash_init(void) {

}

uint32_t internal_flash_get_block_count(void) {
    return ((uint32_t) __fatfs_flash_length) / FLASH_API_BLOCK_SIZE;
}

// TODO support flashing with SD enabled
void internal_flash_hal_erase (uint32_t addr) {
    nrf_nvmc_page_erase(log2phy_addr(addr));
}

void internal_flash_hal_program (uint32_t dst, const void * src, uint32_t len) {
    nrf_nvmc_write_words(log2phy_addr(dst), (uint32_t *) src, len / sizeof(uint32_t));
}

void internal_flash_hal_read (void* dst, uint32_t src, uint32_t len) {
    memcpy(dst, (void*) log2phy_addr(src), len);
}

