/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Scott Shawcroft for Adafruit Industries
 * Copyright (c) 2020 Jeff Epler for Adafruit Industries
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

// TODO: add description of module here

#include "shared-bindings/capsuleio/__init__.h"
#include "py/runtime.h"
#include "py/objstr.h"
#include <string.h>

// for now only string and none can be saved. in the future it may be tuples, floats, ints (maybe)
STATIC mp_obj_t capsule_load(mp_obj_t payload) {
    if (payload == mp_const_none) {
        capsuleio_load_none();
    } else if (MP_OBJ_IS_STR(payload)){
        GET_STR_DATA_LEN(payload, payload_data, payload_len); // declares locals payload_data, payload_len
        capsule_result_t didwork = capsuleio_load_string(
            payload_data, payload_len
        );
        // check if loading that worked
        if (didwork != CAPSULEIO_OK) {
            mp_raise_ValueError(translate("fudge!"));
        }
    } else {
        mp_raise_TypeError(translate("sprinkles!"));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(capsuleio_load_fnobj, capsule_load);



STATIC mp_obj_t capsule_read(void) {
    mp_obj_t parsed_object;
    switch (capsuleio_capsule.kind) {
        case CAPSULEIO_NONE:
            parsed_object = mp_const_none;
            break;
        case CAPSULEIO_STRING:
            parsed_object =  mp_obj_new_str_copy(
                &mp_type_str,
                (const byte*)&capsuleio_capsule.data,
                strlen((const char*)&capsuleio_capsule.data)
            );
            break;
    }
    return parsed_object;
}
MP_DEFINE_CONST_FUN_OBJ_0(capsuleio_read_fnobj, capsule_read);


STATIC const mp_rom_map_elem_t mp_module_capsuleio_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_capsuleio) },
    { MP_ROM_QSTR(MP_QSTR_load), MP_ROM_PTR(&capsuleio_load_fnobj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&capsuleio_read_fnobj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_capsuleio_globals, mp_module_capsuleio_globals_table);


const mp_obj_module_t capsuleio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_capsuleio_globals,
};
