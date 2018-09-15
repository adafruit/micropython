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
#include "flash_devices.h"

#define _VALID_PIN(n)       (defined(MICROPY_QSPI_DATA##n) && (MICROPY_QSPI_DATA##n != 0xff))

#define QSPI_FLASH_MODE     \
        (( _VALID_PIN(0) && _VALID_PIN(1) && _VALID_PIN(2) && _VALID_PIN(3) ) ? 4 : \
         ( _VALID_PIN(0) && _VALID_PIN(1) ) ? 2 : \
         ( _VALID_PIN(0) ) ? 1 : 0)

#if !QSPI_FLASH_MODE
#error MICROPY_QSPI_DATAn must be defined
#endif

enum {
    QSPI_CMD_RSTEN = 0x66,
    QSPI_CMD_RST = 0x99,
    QSPI_CMD_WRSR = 0x01,
    QSPI_CMD_READID = 0x90
};

// If Flash device is not specified, support all devices in flash_devices.h
#ifdef EXTERNAL_FLASH_DEVICES
const qspi_flash_device_t _flash_devices_arr[] = { EXTERNAL_FLASH_DEVICES };
#else
const qspi_flash_device_t _flash_devices_arr[] = {GD25Q16C, MX25R6435F};
#endif

enum {
    FLASH_DEVICE_COUNT = ARRAY_SIZE(_flash_devices_arr)
};

const qspi_flash_device_t* _flash_dev = NULL;

void qspi_flash_init (void) {
    // Init QSPI flash
    nrfx_qspi_config_t qspi_cfg = {
        .xip_offset = 0,
        .pins = {
            .sck_pin = MICROPY_QSPI_SCK,
            .csn_pin = MICROPY_QSPI_CS,
            .io0_pin = MICROPY_QSPI_DATA0,
            .io1_pin = NRF_QSPI_PIN_NOT_CONNECTED,
            .io2_pin = NRF_QSPI_PIN_NOT_CONNECTED,
            .io3_pin = NRF_QSPI_PIN_NOT_CONNECTED,
            
        },
        .prot_if = {
            .readoc = NRF_QSPI_READOC_FASTREAD,
            .writeoc = NRF_QSPI_WRITEOC_PP,
            .addrmode = NRF_QSPI_ADDRMODE_24BIT,
            .dpmconfig = false
        },
        .phy_if = {
            .sck_freq = NRF_QSPI_FREQ_32MDIV1,
            .sck_delay = 1,    // min time CS must stay high before going low again. in unit of 62.5 ns
            .spi_mode = NRF_QSPI_MODE_0,
            .dpmen = false
        },
        .irq_priority = 7,
    };

#if QSPI_FLASH_MODE > 1
    qspi_cfg.pins.io1_pin = MICROPY_QSPI_DATA1;
    qspi_cfg.prot_if.readoc = NRF_QSPI_READOC_READ2IO;
    qspi_cfg.prot_if.writeoc = NRF_QSPI_WRITEOC_PP2O;
#if QSPI_FLASH_MODE > 2
    qspi_cfg.pins.io2_pin = MICROPY_QSPI_DATA2;
    qspi_cfg.pins.io3_pin = MICROPY_QSPI_DATA3;
    qspi_cfg.prot_if.readoc = NRF_QSPI_READOC_READ4IO;
    qspi_cfg.prot_if.writeoc = NRF_QSPI_WRITEOC_PP4IO;
#endif
#endif

    // No callback for blocking API
    nrfx_qspi_init(&qspi_cfg, NULL, NULL);

    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode = 0,
        .length = 0,
        .io2_level = true,
        .io3_level = true,
        .wipwait = false,
        .wren = false
    };

    // Send reset enable
    cinstr_cfg.opcode = QSPI_CMD_RSTEN;
    cinstr_cfg.length = 1;
    nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);

    // Send reset command
    cinstr_cfg.opcode = QSPI_CMD_RST;
    cinstr_cfg.length = 1;
    nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);

    NRFX_DELAY_US(100);    // wait for flash device to reset

    // Send (Read ID + 3 dummy bytes) + Receive 2 bytes of (Manufacture + Device ID)
    uint8_t dummy[6] = {0};
    uint8_t id_resp[6] = { 0 };
    cinstr_cfg.opcode = QSPI_CMD_READID;
    cinstr_cfg.length = 6;

    // Bug with -nrf_qspi_cinstrdata_get() didn't combine data.
    // https://devzone.nordicsemi.com/f/nordic-q-a/38540/bug-nrf_qspi_cinstrdata_get-didn-t-collect-data-from-both-cinstrdat1-and-cinstrdat0
    nrfx_qspi_cinstr_xfer(&cinstr_cfg, dummy, id_resp);

    // Due to the bug, we collect data manually
    uint8_t dev_id = (uint8_t) NRF_QSPI->CINSTRDAT1;
    uint8_t mfgr_id = (uint8_t) ( NRF_QSPI->CINSTRDAT0 >> 24);

    // Look up the flash device in supported array
    for ( int i = 0; i < FLASH_DEVICE_COUNT; i++ ) {
        // Match ID
        if ( _flash_devices_arr[i].manufacturer_id == mfgr_id && _flash_devices_arr[i].device_id == dev_id ) {
            _flash_dev = &_flash_devices_arr[i];
            break;
        }
    }

    if ( _flash_dev ) {
        // Enable quad mode if needed
#if QSPI_FLASH_MODE == 4
        cinstr_cfg.opcode = QSPI_CMD_WRSR;
        cinstr_cfg.length = 3;
        cinstr_cfg.wipwait = cinstr_cfg.wren = true;
        nrfx_qspi_cinstr_xfer(&cinstr_cfg, &_flash_dev->status_quad_enable, NULL);
#endif
    }
}

bool qspi_flash_detected (void) {
    return _flash_dev != NULL;
}

uint32_t qspi_flash_get_block_count (void) {
    return _flash_dev ? (_flash_dev->total_size / FLASH_API_BLOCK_SIZE) : 0;
}

void qspi_flash_hal_erase (uint32_t addr) {
    if ( !(NRFX_SUCCESS == nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, addr)) ) return;
}

void qspi_flash_hal_program (uint32_t dst, const void* src, uint32_t len) {
    if ( !(NRFX_SUCCESS == nrfx_qspi_write(src, len, dst)) ) return;
}

void qspi_flash_hal_read (void* dst, uint32_t src, uint32_t len) {
    nrfx_qspi_read(dst, len, src);
}
