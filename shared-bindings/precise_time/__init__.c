/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Margaret Sy
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

#include "shared-bindings/precise_time/__init__.h"

//| :mod: `precise_time` --- integer time and timing related functions
//| ====================================================================
//|
//| .. module:: precise_time
//|    :synopsis: integer time related functions
//|    :platform: SAMD21
//|
//| The `precise_time` module 
//|
//| .. method:: monotonic()
//|
//|   Returns an always increasing value of time with an unknown reference
//|   point. Only use it to compare against other values from `monotonic`.
//|   Unlike time.monotonic, which is a float, the precision of this does not
//|   degrade over time.
//|
//|   :return: the current monotonic time
//|   :rtype: int
//|
STATIC mp_obj_t precise_time_monotonic(void) {
    return common_hal_time_monotonic();
}
MP_DEFINE_CONST_FUN_OBJ_0(precise_time_monotonic_obj, precise_time_monotonic)

STATIC const mp_rom_map_elem_t precise_time_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR__name__), MP_ROM_QSTR(MP_QSTR_precise_time) },

    { MP_ROM_QSTR(MP_QSTR_monotonic), MP_ROM_PTR(&precise_time_monotonic_obj) }
};

STATIC MP_DEFINE_CONST_DICT(precise_time_module_globals, precise_time_module_globals_table);

const mp_obj_module_t precise_time_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&time_module_globals,
};
