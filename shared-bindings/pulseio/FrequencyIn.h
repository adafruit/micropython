/*
 * This file is part of the Micro Python project, http://micropython.org/
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

#ifndef MICROPY_INCLUDED_SHARED_BINDINGS_PULSEIO_FREQUENCYIN_H
#define MICROPY_INCLUDED_SHARED_BINDINGS_PULSEIO_FREQUENCYIN_H

#include "common-hal/microcontroller/Pin.h"
#include "common-hal/pulseio/FrequencyIn.h"

extern const mp_obj_type_t pulseio_frequencyin_type;

extern void common_hal_pulseio_frequencyin_construct(pulseio_frequencyin_obj_t* self,
    const mcu_pin_obj_t* pin);
extern void common_hal_pulseio_frequencyin_deinit(pulseio_frequencyin_obj_t* self);
extern bool common_hal_pulseio_frequencyin_deinited(pulseio_frequencyin_obj_t* self);
extern void common_hal_pulseio_frequencyin_pause(pulseio_frequencyin_obj_t* self);
extern void common_hal_pulseio_frequencyin_resume(pulseio_frequencyin_obj_t* self);
extern void common_hal_pulseio_frequencyin_clear(pulseio_frequencyin_obj_t* self);
//extern uint16_t common_hal_pulseio_frequencyin_popleft(pulseio_frequencyin_obj_t* self);
//extern uint16_t common_hal_pulseio_frequencyin_get_maxlen(pulseio_frequencyin_obj_t* self);
//extern uint16_t common_hal_pulseio_frequencyin_get_len(pulseio_frequencyin_obj_t* self);
extern uint32_t common_hal_pulseio_frequencyin_get_item(pulseio_frequencyin_obj_t* self);

#endif // MICROPY_INCLUDED_SHARED_BINDINGS_PULSEIO_FREQUENCYIN_H
