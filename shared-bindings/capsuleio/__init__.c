/*
 * This file is part of the Micro Python project, http://micropython.org/
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

// TODO: add description of module

#include "py/objstr.h"
#include "py/runtime.h"
#include "shared-bindings/capsuleio/__init__.h"

// for now only string and none can be saved. in the future it may be tuples, floats, ints (maybe)
STATIC mp_obj_t capsule_bury(mp_obj_t obj) {
    capsule_result_t result = capsuleio_bury_obj(obj);
    switch (result) {
        case CAPSULEIO_STRING_TO_LONG:
            mp_raise_ValueError(translate("too long to store in time capsule"));
            break; // is this needed? the above is noreturn
        case CAPSULEIO_TYPE_CANNOT_BE_BURIED:
            mp_raise_TypeError(translate("can only save a string or None in the time capsule"));
            break; // is this needed? the above is noreturn
        case CAPSULEIO_OK:
        default:
            return mp_const_none;
    }
}

MP_DEFINE_CONST_FUN_OBJ_1(capsuleio_bury_fnobj, capsule_bury);
// this function requires no runtime wrapper
MP_DEFINE_CONST_FUN_OBJ_0(capsuleio_unearth_fnobj, capsuleio_unearth_new_obj);

STATIC const mp_rom_map_elem_t mp_module_capsuleio_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_capsuleio) },
    { MP_ROM_QSTR(MP_QSTR_bury), MP_ROM_PTR(&capsuleio_bury_fnobj) },
    { MP_ROM_QSTR(MP_QSTR_unearth), MP_ROM_PTR(&capsuleio_unearth_fnobj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_capsuleio_globals, mp_module_capsuleio_globals_table);


const mp_obj_module_t capsuleio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_capsuleio_globals,
};
