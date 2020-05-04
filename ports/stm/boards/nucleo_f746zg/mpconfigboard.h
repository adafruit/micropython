/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Lucian Copeland for Adafruit Industries
 * Copyright (c) 2020 Mark Olsson <mark@markolsson.se>
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

#include "devicetree.h"

#define MICROPY_HW_BOARD_NAME       "NUCLEO STM32F746"
#define MICROPY_HW_MCU_NAME         "STM32F746"

#define FLASH_SIZE                  1024 * DT_REG_SIZE(DT_INST(0, st_stm32_flash_controller))
#define FLASH_PAGE_SIZE             (0x4000)

#define BOARD_OSC_DIV (8)

#define DEBUG_UART USART3
#define DEBUG_UART_TX_CLK_ENABLE __HAL_RCC_GPIOD_CLK_ENABLE
#define DEBUG_UART_TX_PORT GPIOD
#define DEBUG_UART_TX_PIN GPIO_PIN_8
#define DEBUG_UART_TX_AF GPIO_AF7_USART3
#define DEBUG_UART_RX_CLK_ENABLE __HAL_RCC_GPIOD_CLK_ENABLE
#define DEBUG_UART_RX_PORT GPIOD
#define DEBUG_UART_RX_PIN GPIO_PIN_9
#define DEBUG_UART_RX_AF GPIO_AF7_USART3
#define DEBUG_UART_IRQn USART3_IRQn
#define DEBUG_UART_CLK_ENABLE __HAL_RCC_USART3_CLK_ENABLE

