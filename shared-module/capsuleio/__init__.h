/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Jonah Yolles-Murphy (TG-Techie)
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

#ifndef MICROPY_INCLUDED_SHARED_MODULE_CAPSULEIO_H
#define MICROPY_INCLUDED_SHARED_MODULE_CAPSULEIO_H

#include "py/obj.h"
#include "py/proto.h"
#include "py/objstr.h"

// TODO(tg-techie): make this settable on a per-board basis
// this is the number of bytes allocated globally for storing capsuleio objects
#define CIRCUITPY_CAPSULEIO_AMOUNT_BYTES 1024

typedef enum {
    CAPSULEIO_OK,
    CAPSULEIO_STRING_TO_LONG,
    CAPSULEIO_TYPE_CANNOT_BE_BURIED,
} capsule_result_t;

// yes this is a manual tagged union
//  couldn't figure out a way to make void feilds in unions
typedef enum  {
    CAPSULEIO_NONE = 0, // the none kind must be zero
    CAPSULEIO_STRING,
} capsule_type_kind_t;

typedef struct {
    capsule_type_kind_t kind;
    // a run-time tag to keep track of the stored type
    byte data[CIRCUITPY_CAPSULEIO_AMOUNT_BYTES];
    // the stored data
    const byte _null_terminator;
    // this *shouldn't* ever be overwritten, so be careful
} capsuleio_capsule_t;

extern capsuleio_capsule_t capsuleio_capsule;

mp_obj_t capsuleio_unearth_new_obj(void);
capsule_result_t capsuleio_bury_obj(mp_obj_t obj);

#endif // MICROPY_INCLUDED_SHARED_MODULE_CAPSULEIO_H
