/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Noralf Trønnes
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

#include <stdint.h>

#include "lib/utils/context_manager_helpers.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "py/runtime0.h"
#include "py/stream.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/pulseio/Counter.h"
#include "shared-bindings/util.h"
#include "supervisor/shared/translate.h"

//| .. currentmodule:: pulseio
//|
//| :class:`Counter` -- Count pulses
//| ================================
//|
//| Counter is used to count edges on a pulse train.
//|
//| .. class:: Counter(pin, ...)
//|
//|   Create a Counter object associated with the given pin.
//|
//|   :param ~microcontroller.Pin pin: Pin to read pulses from.
//|
STATIC mp_obj_t pulseio_counter_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *pos_args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, pos_args + n_args);
    enum { ARG_pin, ARG_edges, ARG_pull };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_pin, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_edges, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, &kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    assert_pin(args[ARG_pin].u_obj, false);
    const mcu_pin_obj_t* pin = MP_OBJ_TO_PTR(args[ARG_pin].u_obj);
    assert_pin_free(pin);

    pulseio_counter_obj_t *self = m_new_obj(pulseio_counter_obj_t);
    self->base.type = &pulseio_counter_type;

    common_hal_pulseio_counter_construct(self, pin);

    return MP_OBJ_FROM_PTR(self);
}

//|   .. method:: deinit()
//|
//|      Deinitialises the Counter and releases any hardware resources for reuse.
//|
STATIC mp_obj_t pulseio_counter_deinit(mp_obj_t self_in) {
    pulseio_counter_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_pulseio_counter_deinit(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pulseio_counter_deinit_obj, pulseio_counter_deinit);

//|   .. method:: __enter__()
//|
//|      No-op used by Context Managers.
//|
//  Provided by context manager helper.

//|   .. method:: __exit__()
//|
//|      Automatically deinitializes the hardware when exiting a context. See
//|      :ref:`lifetime-and-contextmanagers` for more info.
//|
STATIC mp_obj_t pulseio_counter_obj___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    common_hal_pulseio_counter_deinit(args[0]);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pulseio_counter___exit___obj, 4, 4, pulseio_counter_obj___exit__);

//|   .. method:: clear()
//|
//|     Clears counter and returns its value.
//|
STATIC mp_obj_t pulseio_counter_obj_clear(mp_obj_t self_in) {
    pulseio_counter_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_counter_deinited(self));

    return mp_obj_new_int_from_uint(common_hal_pulseio_counter_get_count(self, true));
}
MP_DEFINE_CONST_FUN_OBJ_1(pulseio_counter_clear_obj, pulseio_counter_obj_clear);

STATIC mp_obj_t pulseio_counter_get_count(mp_obj_t self_in) {
    pulseio_counter_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_counter_deinited(self));

    return mp_obj_new_int_from_uint(common_hal_pulseio_counter_get_count(self, false));
}
MP_DEFINE_CONST_PROP_GET(pulseio_counter_count_obj, pulseio_counter_get_count);

STATIC mp_obj_t pulseio_counter_get_pinvalue(mp_obj_t self_in) {
    pulseio_counter_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_counter_deinited(self));

    return mp_obj_new_int_from_uint(common_hal_pulseio_counter_get_pinvalue(self));
}
MP_DEFINE_CONST_PROP_GET(pulseio_counter_pinvalue_obj, pulseio_counter_get_pinvalue);

STATIC mp_uint_t pulseio_counter_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    pulseio_counter_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_counter_deinited(self));

    if (request != MP_STREAM_POLL) {
        *errcode = MP_EINVAL;
        return MP_STREAM_ERROR;
    }

    uint32_t count = common_hal_pulseio_counter_get_count(self, false);
    return count > 0 ? 1 : 0;
}

STATIC const mp_stream_p_t pulseio_counter_p = {
    .ioctl = pulseio_counter_ioctl,
};

STATIC const mp_rom_map_elem_t pulseio_counter_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pulseio_counter_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&default___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&pulseio_counter___exit___obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&pulseio_counter_clear_obj) },

    // Properties
    { MP_ROM_QSTR(MP_QSTR_count), MP_ROM_PTR(&pulseio_counter_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_pinvalue), MP_ROM_PTR(&pulseio_counter_pinvalue_obj) },
};
STATIC MP_DEFINE_CONST_DICT(pulseio_counter_locals_dict, pulseio_counter_locals_dict_table);

const mp_obj_type_t pulseio_counter_type = {
    { &mp_type_type },
    .name = MP_QSTR_PulseIn,
    .make_new = pulseio_counter_make_new,
    .protocol = &pulseio_counter_p,
    .locals_dict = (mp_obj_dict_t*)&pulseio_counter_locals_dict,
};
