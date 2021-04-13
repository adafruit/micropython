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

#include "shared-bindings/capsuleio/__init__.h"
#include "py/objstr.h"
#include <string.h>


// the time capsule memory reservation
capsuleio_capsule_t capsuleio_capsule;

STATIC bool capsuleio_bury_string(const byte *data, size_t length) {
    // the only fail mode is the string is too long
    // make sure the data will fit
    if (length > CIRCUITPY_CAPSULEIO_AMOUNT_BYTES) {
        return false;
    } else {
        // set the type of the data stored
        capsuleio_capsule.kind = CAPSULEIO_STRING;
        // copy the string data in
        memcpy(&capsuleio_capsule.data, data, length);
        // write the null byte
        capsuleio_capsule.data[length] = 0;
        return true;
    }
}

STATIC void capsuleio_bury_none(void) {
    capsuleio_capsule.kind = CAPSULEIO_NONE;
    memset(&capsuleio_capsule.data, 0, CIRCUITPY_CAPSULEIO_AMOUNT_BYTES);
    return;
}

mp_obj_t capsuleio_unearth_new_obj(void) {
    switch (capsuleio_capsule.kind) {
        case CAPSULEIO_NONE:
            return mp_const_none;
        case CAPSULEIO_STRING:
            return mp_obj_new_str_copy(
                &mp_type_str,
                (const byte *)&capsuleio_capsule.data,
                strlen((const char *)&capsuleio_capsule.data));
        default:
            // this should never be reached, but just in case
            return mp_const_none;
    }
}

capsule_result_t capsuleio_bury_obj(mp_obj_t obj) {
    if (obj == mp_const_none) {
        capsuleio_bury_none();
        return CAPSULEIO_OK;
    } else if (MP_OBJ_IS_STR(obj)) {
        GET_STR_DATA_LEN(obj, data, length); // defines locals data and length
        bool bury_worked = capsuleio_bury_string(data, length);
        if (!bury_worked) {
            return CAPSULEIO_STRING_TO_LONG;
        } else {
            return CAPSULEIO_OK;
        }
    } else {
        return CAPSULEIO_TYPE_CANNOT_BE_BURIED;
    }
}
