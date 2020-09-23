/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Scott Shawcroft for Adafruit Industries
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
#include "mpconfigboard.h"
#include "shared-bindings/microcontroller/Pin.h"

void board_init(void)
{
    // USB
    common_hal_never_reset_pin(&pin_GPIO19);
    common_hal_never_reset_pin(&pin_GPIO20);

    // Debug UART
    common_hal_never_reset_pin(&pin_GPIO43);
    common_hal_never_reset_pin(&pin_GPIO44);

    // SPI Flash and RAM
    common_hal_never_reset_pin(&pin_GPIO26);
    common_hal_never_reset_pin(&pin_GPIO27);
    common_hal_never_reset_pin(&pin_GPIO28);
    common_hal_never_reset_pin(&pin_GPIO29);
    common_hal_never_reset_pin(&pin_GPIO30);
    common_hal_never_reset_pin(&pin_GPIO31);
    common_hal_never_reset_pin(&pin_GPIO32);
}

bool board_requests_safe_mode(void)
{
    return false;
}

void reset_board(void)
{
}
