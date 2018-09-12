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

#define NO_CACHE        0xffffffff

uint8_t _flash_cache[FLASH_API_PAGE_SIZE] __attribute__((aligned(4)));
uint32_t _flash_page_addr = NO_CACHE;

static inline uint32_t lba2addr(uint32_t block) {
    return ((uint32_t) __fatfs_flash_start_addr) + block * FLASH_API_BLOCK_SIZE;
}

void internal_flash_init(void) {

}

uint32_t internal_flash_get_block_size(void) {
    return FLASH_API_BLOCK_SIZE;
}

uint32_t internal_flash_get_block_count(void) {
    return ((uint32_t) __fatfs_flash_length) / FLASH_API_BLOCK_SIZE;
}

// TODO support flashing with SD enabled
void internal_flash_flush(void) {
    if (_flash_page_addr == NO_CACHE) return;

    // Skip if data is the same
    if ( memcmp(_flash_cache, (void *) _flash_page_addr, FLASH_API_PAGE_SIZE) != 0 ) {
//        _is_flashing = true;
        nrf_nvmc_page_erase(_flash_page_addr);
        nrf_nvmc_write_words(_flash_page_addr, (uint32_t *) _flash_cache, FLASH_API_PAGE_SIZE / sizeof(uint32_t));
    }

    _flash_page_addr = NO_CACHE;
}

mp_uint_t internal_flash_read_blocks(uint8_t *dest, uint32_t block, uint32_t num_blocks) {
    uint32_t src = lba2addr(block);
    memcpy(dest, (uint8_t*) src, FLASH_API_BLOCK_SIZE * num_blocks);
    return 0; // success
}

mp_uint_t internal_flash_write_blocks (const uint8_t *src, uint32_t lba, uint32_t num_blocks) {
    while (num_blocks) {
        const uint32_t addr = lba2addr(lba);
        const uint32_t page_addr = addr & ~(FLASH_API_PAGE_SIZE - 1);

        uint32_t count = 8 - (lba % 8); // up to page boundary
        count = MIN(num_blocks, count);

        if (page_addr != _flash_page_addr) {
            internal_flash_flush();

            // writing previous cached data, skip current data until flashing is done
            // tinyusb stack will invoke write_block() with the same parameters later on
            //        if ( _is_flashing ) return;

            _flash_page_addr = page_addr;
            memcpy(_flash_cache, (void *) page_addr, FLASH_API_PAGE_SIZE);
        }

        memcpy(_flash_cache + (addr & (FLASH_API_PAGE_SIZE - 1)), src, count * FLASH_API_BLOCK_SIZE);

        // adjust for next run
        lba += count;
        src += count * FLASH_API_BLOCK_SIZE;
        num_blocks -= count;
    }

    return 0; // success
}
