/*
 * This file is part of the Micro Python project, http://micropython.org/
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
#include "rotation.h"

const mp_obj_type_t display_rotation_type;

const display_rotation_obj_t display_rotation_0_obj = {
    { &display_rotation_type },
};

const display_rotation_obj_t display_rotation_90_obj = {
    { &display_rotation_type },
};

const display_rotation_obj_t display_rotation_180_obj = {
    { &display_rotation_type },
};

const display_rotation_obj_t display_rotation_270_obj = {
    { &display_rotation_type },
};

STATIC const mp_rom_map_elem_t rotation_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_R0),    MP_ROM_PTR(&display_rotation_0_obj) },
    { MP_ROM_QSTR(MP_QSTR_R90),   MP_ROM_PTR(&display_rotation_90_obj) },
    { MP_ROM_QSTR(MP_QSTR_R180),  MP_ROM_PTR(&display_rotation_180_obj) },
    { MP_ROM_QSTR(MP_QSTR_R270),  MP_ROM_PTR(&display_rotation_270_obj) },
};
STATIC MP_DEFINE_CONST_DICT(rotation_locals_dict, rotation_locals_dict_table);

STATIC void rotation_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    qstr rotation = MP_QSTR_R0;
    if (MP_OBJ_TO_PTR(self_in) == MP_ROM_PTR(&display_rotation_90_obj)) {
        rotation = MP_QSTR_R90;
    } else if (MP_OBJ_TO_PTR(self_in) == MP_ROM_PTR(&display_rotation_180_obj)) {
        rotation = MP_QSTR_R180;
    } else if (MP_OBJ_TO_PTR(self_in) == MP_ROM_PTR(&display_rotation_270_obj)) {
        rotation = MP_QSTR_R270;
    }

    mp_printf(print, "%q.%q.%q", MP_QSTR_display, MP_QSTR_Rotation, rotation);
}

const mp_obj_type_t display_rotation_type = {
    { &mp_type_type },
    .name = MP_QSTR_Rotation,
    .print = rotation_print,
    .locals_dict = (mp_obj_t)&rotation_locals_dict,
};
