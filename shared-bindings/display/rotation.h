/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Artur Pacholec
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

#ifndef MICROPY_INCLUDED_DISPLAY_ROTATION_H
#define MICROPY_INCLUDED_DISPLAY_ROTATION_H

#include "py/obj.h"

typedef enum {
    ROTATION_0 = 0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270
} display_rotation_t;

typedef struct {
    mp_obj_base_t base;
} display_rotation_obj_t;

const mp_obj_type_t display_rotation_type;

extern const display_rotation_obj_t display_rotation_0_obj;
extern const display_rotation_obj_t display_rotation_90_obj;
extern const display_rotation_obj_t display_rotation_180_obj;
extern const display_rotation_obj_t display_rotation_270_obj;

#endif  // MICROPY_INCLUDED_DISPLAY_ROTATION_H
