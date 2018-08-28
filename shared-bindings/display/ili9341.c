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

#include "ili9341.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "rotation.h"
#include "shared-module/display/ili9341.h"

#define DISP_WIDTH  320
#define DISP_HEIGHT 240

STATIC mp_obj_t ili9341_make_new(const mp_obj_type_t *type, size_t n_args,
        size_t n_kw, const mp_obj_t *args) {

    ili9341_obj_t *self = m_new_obj(ili9341_obj_t);
    self->base.type = &display_ili9341_type;
    self->rotation = ROTATION_0;
    self->spi = MP_OBJ_TO_PTR(args[0]);
    self->dc = MP_OBJ_TO_PTR(args[1]);
    self->cs = MP_OBJ_TO_PTR(args[2]);

    display_init9341_write_init(self);

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t ili9341_pixel(size_t n_args, const mp_obj_t *args) {
    ili9341_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    const uint16_t x = mp_obj_get_int(args[1]);
    const uint16_t y = mp_obj_get_int(args[2]);
    const uint32_t color = mp_obj_get_int(args[3]);

    display_ili9341_pixel(self, x, y, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ili9341_pixel_obj, 4, 4, ili9341_pixel);

STATIC mp_obj_t ili9341_fill(mp_obj_t self_in, mp_obj_t color_in) {
    ili9341_obj_t *self = MP_OBJ_TO_PTR(self_in);

    const uint32_t color = mp_obj_get_int(color_in);

    display_ili9341_rect(self, 0, 0, DISP_WIDTH, DISP_HEIGHT, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ili9341_fill_obj, ili9341_fill);

STATIC mp_obj_t ili9341_fill_rectangle(size_t n_args, const mp_obj_t *args) {
    ili9341_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    const uint16_t x = mp_obj_get_int(args[1]);
    const uint16_t y = mp_obj_get_int(args[2]);
    const uint16_t width = mp_obj_get_int(args[3]);
    const uint16_t height = mp_obj_get_int(args[4]);
    const uint32_t color = mp_obj_get_int(args[5]);

    display_ili9341_rect(self, x, y, width, height, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ili9341_fill_rectangle_obj, 6, 6, ili9341_fill_rectangle);

STATIC mp_obj_t ili9341_obj_get_rotation(mp_obj_t self_in) {
    ili9341_obj_t *self = MP_OBJ_TO_PTR(self_in);

    switch (self->rotation) {
        case ROTATION_0:   return (mp_obj_t)&display_rotation_0_obj;
        case ROTATION_90:  return (mp_obj_t)&display_rotation_90_obj;
        case ROTATION_180: return (mp_obj_t)&display_rotation_180_obj;
        case ROTATION_270: return (mp_obj_t)&display_rotation_270_obj;
    }

    return (mp_obj_t)&mp_const_none_obj;
}
MP_DEFINE_CONST_FUN_OBJ_1(ili9341_get_rotation_obj, ili9341_obj_get_rotation);

STATIC mp_obj_t ili9341_obj_set_rotation(mp_obj_t self_in, mp_obj_t rotation_obj) {
    ili9341_obj_t *self = MP_OBJ_TO_PTR(self_in);

    display_rotation_t rotation = ROTATION_0;
    if (rotation_obj == &display_rotation_0_obj) {
        rotation = ROTATION_0;
    } else if (rotation_obj == &display_rotation_90_obj) {
        rotation = ROTATION_90;
    } else if (rotation_obj == &display_rotation_180_obj) {
        rotation = ROTATION_180;
    } else if (rotation_obj == &display_rotation_270_obj) {
        rotation = ROTATION_270;
    } else if (rotation_obj != mp_const_none) {
        mp_raise_ValueError("Unsupported rotation value.");
    }

    display_ili9341_set_rotation(self, rotation);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(ili9341_set_rotation_obj, ili9341_obj_set_rotation);

const mp_obj_property_t ili9341_rotation_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&ili9341_get_rotation_obj,
              (mp_obj_t)&ili9341_set_rotation_obj,
              (mp_obj_t)&mp_const_none_obj},
};

STATIC const mp_rom_map_elem_t ili9341_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_fill),           MP_ROM_PTR(&ili9341_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill_rectangle), MP_ROM_PTR(&ili9341_fill_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel),          MP_ROM_PTR(&ili9341_pixel_obj) },

    { MP_ROM_QSTR(MP_QSTR_rotation),       MP_ROM_PTR(&ili9341_rotation_obj) },
};
STATIC MP_DEFINE_CONST_DICT(ili9341_locals_dict, ili9341_locals_dict_table);

const mp_obj_type_t display_ili9341_type = {
    { &mp_type_type },
    .name = MP_QSTR_ILI9341,
    .make_new = ili9341_make_new,
    .locals_dict = (mp_obj_dict_t*)&ili9341_locals_dict,
};
