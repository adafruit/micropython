/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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

#include "py/mphal.h"
#include <string.h>
#include "supervisor/debug.h"
#include "common-hal/microcontroller/Pin.h"
#include "lib/utils/interrupt_char.h"
#include "lib/mp-readline/readline.h"
#include STM32_HAL_H

UART_HandleTypeDef huart;
uint8_t debug_rx_char;

void debug_rxcpltcallback(UART_HandleTypeDef *handle) {
    if ((HAL_UART_GetState(handle) & HAL_UART_STATE_BUSY_RX) == HAL_UART_STATE_BUSY_RX) {
        return;
    }
     if (debug_rx_char == CHAR_CTRL_C) {
        debug_disable_interrupt();
        mp_keyboard_interrupt();
        return;
    }
    HAL_UART_Receive_IT(handle, &debug_rx_char, 1);
}

void debug_enable_interrupt(void) {
    HAL_UART_RegisterCallback(&huart, HAL_UART_RX_COMPLETE_CB_ID, debug_rxcpltcallback);
    HAL_NVIC_DisableIRQ(DEBUG_UART_IRQn); //prevent handle lock contention
    HAL_UART_Receive_IT(&huart, &debug_rx_char, 1);
    HAL_NVIC_SetPriority(DEBUG_UART_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DEBUG_UART_IRQn);
    __HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);
}

void debug_disable_interrupt(void) {
    HAL_UART_UnRegisterCallback(&huart, HAL_UART_RX_COMPLETE_CB_ID);
    __HAL_UART_DISABLE_IT(&huart, UART_IT_RXNE);
    HAL_NVIC_DisableIRQ(DEBUG_UART_IRQn);
    HAL_UART_AbortReceive(&huart);
}

void debug_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    DEBUG_UART_CLK_ENABLE();
    DEBUG_UART_TX_CLK_ENABLE();
    DEBUG_UART_RX_CLK_ENABLE();

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3; // TODO
    PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1; // TODO
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    GPIO_InitStruct.Pin = DEBUG_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = DEBUG_UART_TX_AF;
    HAL_GPIO_Init(DEBUG_UART_TX_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = DEBUG_UART_RX_PIN;
    GPIO_InitStruct.Alternate = DEBUG_UART_RX_AF;
    HAL_GPIO_Init(DEBUG_UART_RX_PORT, &GPIO_InitStruct);

    never_reset_pin_number(3, 8); // TODO
    never_reset_pin_number(3, 9); // TODO

    huart.Instance = DEBUG_UART;
    huart.Init.BaudRate = 115200;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart);

    debug_enable_interrupt();
}

char debug_read(void) {
    uint8_t data;
    HAL_StatusTypeDef ret;
    ret = HAL_UART_Receive(&huart, &data, 1, 500);
    if (ret != HAL_OK) {
        return 0;
    }
    return data;
}

bool debug_bytes_available(void) {
    return __HAL_UART_GET_FLAG(&huart, UART_FLAG_RXNE);
}

void debug_write_substring(const char *text, uint32_t len) {
    if (len == 0) {
        return;
    }
    HAL_UART_Transmit(&huart, (uint8_t*)text, len, 5000);
}

void debug_irq_handler(void) {
    HAL_NVIC_ClearPendingIRQ(DEBUG_UART_IRQn);
    HAL_UART_IRQHandler(&huart);
}
