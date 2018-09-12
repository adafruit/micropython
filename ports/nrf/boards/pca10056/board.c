/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries
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

#include "boards/board.h"
#include "nrfx_qspi.h"

#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99
#define QSPI_STD_CMD_WRSR   0x01

extern void qspi_flash_complete (void);

void qflash_hdl (nrfx_qspi_evt_t event, void * p_context)
{
    (void) p_context;
    (void) event;

//    qspi_flash_complete();
}


void board_init(void) {
    // Init QSPI flash
    nrfx_qspi_config_t qspi_cfg = {
        .xip_offset = 0,
        .pins = {
            .sck_pin = 19,
            .csn_pin = 17,
            .io0_pin = 20,
            .io1_pin = 21,
            .io2_pin = 22,
            .io3_pin = 23,
        },
        .prot_if = {
            .readoc = NRF_QSPI_READOC_READ4IO,
            .writeoc = NRF_QSPI_WRITEOC_PP4IO,
            .addrmode = NRF_QSPI_ADDRMODE_24BIT,
            .dpmconfig = false,    // deep power down
        },
        .phy_if = {
            .sck_freq = NRF_QSPI_FREQ_32MDIV1,
            .sck_delay = 1,
            .spi_mode = NRF_QSPI_MODE_0,
            .dpmen = false
        },
        .irq_priority = 7,
    };

//    nrfx_qspi_init(&qspi_cfg, qflash_hdl, NULL);
    nrfx_qspi_init(&qspi_cfg, NULL, NULL);

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

bool board_requests_safe_mode(void) {
  return false;
}

void reset_board(void) {

}
