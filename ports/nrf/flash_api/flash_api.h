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

#ifndef FLASH_API_H_
#define FLASH_API_H_

#include "extmod/vfs_fat.h"

#define FLASH_API_BLOCK_SIZE        FILESYSTEM_BLOCK_SIZE
#define FLASH_API_PAGE_SIZE         4096
#define FLASH_API_BLOCK_PER_PAGE    (FLASH_API_PAGE_SIZE/FLASH_API_BLOCK_SIZE)

enum {
    FLASH_STATE_IDLE,
    FLASH_STATE_BUSY,
    FLASH_STATE_COMPLETE
};

#ifdef __cplusplus
 extern "C" {
#endif

// Low level flash hal
void flash_init (void);
uint32_t flash_get_block_count (void);
void flash_hal_erase (uint32_t addr);
void flash_hal_program (uint32_t dst, const void* src, uint32_t len);
void flash_hal_read (void* dst, uint32_t src, uint32_t len);

void flash_init_vfs (struct _fs_user_mount_t *vfs);

mp_uint_t flash_read_blocks (uint8_t* dst, uint32_t lba, uint32_t count);
mp_uint_t flash_write_blocks (const uint8_t *src, uint32_t lba, uint32_t count);
void flash_flush (void);


#ifdef __cplusplus
 }
#endif

#endif /* FLASH_API_H_ */
