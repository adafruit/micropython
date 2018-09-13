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
#include "nrf_gpio.h"
#include "flash_api.h"
#include "internal_flash.h"
#include "qspi_flash.h"

#include "py/mphal.h"
#include "py/obj.h"
#include "py/runtime.h"

//--------------------------------------------------------------------+
// Block API with caching
//--------------------------------------------------------------------+
#define NO_CACHE 0xffffffff

static uint32_t _cache_addr = NO_CACHE;
static uint8_t _cache_buf[FLASH_API_PAGE_SIZE] __attribute__((aligned(4)));

static inline uint32_t page_addr_of (uint32_t addr) {
    return addr & ~(FLASH_API_PAGE_SIZE - 1);
}

static inline uint32_t page_offset_of (uint32_t addr) {
    return addr & (FLASH_API_PAGE_SIZE - 1);
}

mp_uint_t flash_read_blocks (uint8_t* dst, uint32_t lba, uint32_t count) {
    flash_hal_read(dst, lba * FLASH_API_BLOCK_SIZE, count * FLASH_API_BLOCK_SIZE);
    return 0;
}

mp_uint_t flash_write_blocks (const uint8_t *src, uint32_t lba, uint32_t num_blocks) {
    uint32_t dst = lba * FLASH_API_BLOCK_SIZE;

    // Program blocks up to page boundary each loop
    while ( num_blocks ) {
        const uint32_t page_addr = page_addr_of(dst);
        const uint32_t offset = page_offset_of(dst);

        uint32_t wr_blocks = FLASH_API_BLOCK_PER_PAGE - (lba % FLASH_API_BLOCK_PER_PAGE);    // up to page boundary
        wr_blocks = MIN(num_blocks, wr_blocks);

        const uint32_t wr_bytes = wr_blocks * FLASH_API_BLOCK_SIZE;

        // Page changes, flush old and update new cache
        if ( page_addr != _cache_addr ) {
            flash_flush();
            _cache_addr = page_addr;

            // read existing flash to cache except those we are writing
            if ( offset ) {
                flash_hal_read(_cache_buf, page_addr, offset);
            }

            const uint32_t last_byte = offset + wr_bytes;
            if ( last_byte < FLASH_API_PAGE_SIZE ) {
                flash_hal_read(_cache_buf + last_byte, page_addr + last_byte, FLASH_API_PAGE_SIZE - last_byte);
            }
        }

        memcpy(_cache_buf + offset, src, wr_bytes);

        // adjust for next run
        dst += wr_bytes;
        src += wr_bytes;
        num_blocks -= wr_blocks;
    }

    return 0;
}

void flash_flush (void) {
    if ( _cache_addr == NO_CACHE ) return;

    flash_hal_erase(_cache_addr);
    flash_hal_program(_cache_addr, _cache_buf, FLASH_API_PAGE_SIZE);

    _cache_addr = NO_CACHE;
}

uint32_t flash_read_blocks_nonblocking (uint8_t* dst, uint32_t lba, uint32_t count) {
    return count;
}

//--------------------------------------------------------------------+
// Flash HAL
//--------------------------------------------------------------------+

#ifdef QSPI_FLASH_FILESYSTEM

void flash_init (void) {
    qspi_flash_init();
}

uint32_t flash_get_block_count (void) {
    return qspi_flash_get_block_count();
}

void flash_hal_erase (uint32_t addr) {
    qspi_flash_hal_erase(addr);
}

void flash_hal_program (uint32_t dst, const void * src, uint32_t len) {
    qspi_flash_hal_program(dst, src, len);
}

void flash_hal_read (void* dst, uint32_t src, uint32_t len) {
    qspi_flash_hal_read(dst, src, len);
}

#else

void flash_init (void) {
    internal_flash_init();
}

uint32_t flash_get_block_count (void) {
    return internal_flash_get_block_count();
}

void flash_hal_erase (uint32_t addr) {
    internal_flash_hal_erase(addr);
}

void flash_hal_program (uint32_t dst, const void * src, uint32_t len) {
    internal_flash_hal_program(dst, src, len);
}

void flash_hal_read (void* dst, uint32_t src, uint32_t len) {
    internal_flash_hal_read(dst, src, len);
}

#endif

/******************************************************************************/
// MicroPython bindings
//
// Expose the flash as an object with the block protocol.
// there is a singleton Flash object
/******************************************************************************/
extern const struct _mp_obj_type_t _flash_type;    // forward declaration

const STATIC mp_obj_base_t _flash_obj = { &_flash_type };

STATIC mp_obj_t flash_obj_make_new (const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return singleton object
    return (mp_obj_t) &_flash_obj;
}

STATIC mp_obj_t flash_obj_readblocks (mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    mp_uint_t ret = flash_read_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_API_BLOCK_SIZE);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(flash_obj_readblocks_obj, flash_obj_readblocks);

STATIC mp_obj_t flash_obj_writeblocks (mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    mp_uint_t ret = flash_write_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_API_BLOCK_SIZE);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(flash_obj_writeblocks_obj, flash_obj_writeblocks);

STATIC mp_obj_t flash_obj_ioctl (mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch ( cmd ) {
        case BP_IOCTL_INIT:
            flash_init();
            return MP_OBJ_NEW_SMALL_INT(0);
        case BP_IOCTL_DEINIT:
            flash_flush();
            return MP_OBJ_NEW_SMALL_INT(0);    // TODO properly
        case BP_IOCTL_SYNC:
            flash_flush();
            return MP_OBJ_NEW_SMALL_INT(0);
        case BP_IOCTL_SEC_COUNT:
            return MP_OBJ_NEW_SMALL_INT(flash_get_block_count());
        case BP_IOCTL_SEC_SIZE:
            return MP_OBJ_NEW_SMALL_INT(FLASH_API_BLOCK_SIZE);
        default:
            return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(flash_obj_ioctl_obj, flash_obj_ioctl);

    const STATIC mp_rom_map_elem_t flash_obj_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&flash_obj_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&flash_obj_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&flash_obj_ioctl_obj) },
};

STATIC MP_DEFINE_CONST_DICT(flash_obj_locals_dict, flash_obj_locals_dict_table);

const mp_obj_type_t _flash_type = {
    { &mp_type_type },
    .name = MP_QSTR_Flash,
    .make_new = flash_obj_make_new,
    .locals_dict = (mp_obj_t) &flash_obj_locals_dict,
};

void flash_init_vfs (fs_user_mount_t *vfs) {
    vfs->base.type = &mp_fat_vfs_type;
    vfs->flags |= FSUSER_NATIVE | FSUSER_HAVE_IOCTL;
    vfs->fatfs.drv = vfs;

//    vfs->fatfs.part = 1; // flash filesystem lives on first partition
    vfs->readblocks[0] = (mp_obj_t) &flash_obj_readblocks_obj;
    vfs->readblocks[1] = (mp_obj_t) &_flash_obj;
    vfs->readblocks[2] = (mp_obj_t) flash_read_blocks;    // native version

    vfs->writeblocks[0] = (mp_obj_t) &flash_obj_writeblocks_obj;
    vfs->writeblocks[1] = (mp_obj_t) &_flash_obj;
    vfs->writeblocks[2] = (mp_obj_t) flash_write_blocks;    // native version

    vfs->u.ioctl[0] = (mp_obj_t) &flash_obj_ioctl_obj;
    vfs->u.ioctl[1] = (mp_obj_t) &_flash_obj;

    // Activity LED for flash writes.
#ifdef MICROPY_HW_LED_MSC
    nrf_gpio_cfg_output(MICROPY_HW_LED_MSC);
    nrf_gpio_pin_write(MICROPY_HW_LED_MSC, 1 - MICROPY_HW_LED_MSC_ACTIVE_LEVEL);
#endif
}
