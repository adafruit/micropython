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
#include "nrf_gpio.h"
#include "nrfx_qspi.h"
#include "flash_api.h"
#include "qspi_flash.h"

#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99
#define QSPI_STD_CMD_WRSR   0x01

volatile static bool _qspi_complete = false;

void qspi_flash_isr (nrfx_qspi_evt_t event, void * p_context)
{
    (void) p_context;
    (void) event;

    _qspi_complete = true;
}

void qspi_flash_init (void) {
    // Init QSPI flash
    nrfx_qspi_config_t qspi_cfg = {
        .xip_offset = 0,
        .pins = {
            .sck_pin = QSPI_FLASH_SCK,
            .csn_pin = QSPI_FLASH_CS,
            .io0_pin = QSPI_FLASH_D0,
            .io1_pin = QSPI_FLASH_D1,
            .io2_pin = QSPI_FLASH_D2,
            .io3_pin = QSPI_FLASH_D3,
        },
        .prot_if = {
            .readoc = QSPI_FLASH_READ_OPCODE,
            .writeoc = QSPI_FLASH_WRITE_OPCODE,
            .addrmode = NRF_QSPI_ADDRMODE_24BIT,
            .dpmconfig = false
        },
        .phy_if = {
            .sck_freq = QSPI_FLASH_FREQ,
            .sck_delay = 1,
            .spi_mode = NRF_QSPI_MODE_0,
            .dpmen = false
        },
        .irq_priority = 7,
    };

    nrfx_qspi_init(&qspi_cfg, qspi_flash_isr, NULL);

    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode = 0,
        .length = 0,
        .io2_level = true,
        .io3_level = true,
        .wipwait = true,
        .wren = true
    };

    // Send reset enable
    cinstr_cfg.opcode = QSPI_STD_CMD_RSTEN;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
    nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);

    // Send reset command
    cinstr_cfg.opcode = QSPI_STD_CMD_RST;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
    nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);

    // Switch to qspi mode
    uint8_t sr_quad_en = 0x40;
    cinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    nrfx_qspi_cinstr_xfer(&cinstr_cfg, &sr_quad_en, NULL);
}

uint32_t qspi_flash_get_block_count (void) {
    return QSPI_FLASH_SIZE / FLASH_API_BLOCK_SIZE;
}

uint8_t qspi_flash_get_state (void) {
    return 0;
}

void qspi_flash_hal_erase (uint32_t addr) {
    if ( !(NRFX_SUCCESS == nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, addr)) ) return;
    while ( !_qspi_complete ) {
    }
    _qspi_complete = false;
}

void qspi_flash_hal_program (uint32_t dst, const void* src, uint32_t len) {
    if ( !(NRFX_SUCCESS == nrfx_qspi_write(src, len, dst)) ) return;
    while ( !_qspi_complete ) {
    }
    _qspi_complete = false;
}

void qspi_flash_hal_read (void* dst, uint32_t src, uint32_t len) {
    nrfx_qspi_read(dst, len, src);
    while ( !_qspi_complete ) {
    }
    _qspi_complete = false;
}
