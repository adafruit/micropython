/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Michael Schroeder
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
#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/pulseio/FrequencyIn.h"
#include "shared-bindings/util.h"
#include "peripheral_clk_config.h"

void MP_WEAK common_hal_pulseio_frequencyin_construct(pulseio_frequencyin_obj_t* self,
    const mcu_pin_obj_t* pin) {
    mp_raise_NotImplementedError(translate("FrequencyIn is not supported on this board"));
}
void MP_WEAK common_hal_pulseio_frequencyin_deinit(pulseio_frequencyin_obj_t* self) {
    mp_raise_NotImplementedError(translate("FrequencyIn is not supported on this board"));
}
bool MP_WEAK common_hal_pulseio_frequencyin_deinited(pulseio_frequencyin_obj_t* self) {
    mp_raise_NotImplementedError(translate("FrequencyIn is not supported on this board"));
}
void MP_WEAK common_hal_pulseio_frequencyin_pause(pulseio_frequencyin_obj_t* self) {
    mp_raise_NotImplementedError(translate("FrequencyIn is not supported on this board"));
}
void MP_WEAK common_hal_pulseio_frequencyin_resume(pulseio_frequencyin_obj_t* self){
    mp_raise_NotImplementedError(translate("FrequencyIn is not supported on this board"));
}
void MP_WEAK common_hal_pulseio_frequencyin_clear(pulseio_frequencyin_obj_t* self) {
    mp_raise_NotImplementedError(translate("FrequencyIn is not supported on this board"));
}
uint32_t MP_WEAK common_hal_pulseio_frequencyin_get_item(pulseio_frequencyin_obj_t* self) {
    mp_raise_NotImplementedError(translate("FrequencyIn is not supported on this board"));
}

//| .. currentmodule:: pulseio
//|
//| :class:`FrequencyIn` -- Read a frequency signal
//| ========================================================
//|
//| FrequencyIn is used to measure the frequency, in hertz, of a digital signal
//| on an incoming pin. It will not determine pulse width (use ``PulseIn``).
//|
//| .. class:: FrequencyIn(pin)
//|
//|   Create a FrequencyIn object associated with the given pin.
//|
//|   :param ~microcontroller.Pin pin: Pin to read pulses from.
//|
//|   Read the incoming frequency from a pin::
//|
//|     import pulseio
//|     import board
//|
//|     frequency = pulseio.FrequencyIn(board.D11)
//|
//|     # Loop while printing the detected frequency
//|     while True:
//|         print(frequency.value)
//|
//|         # Optional clear() will reset the value
//|         # to zero. Without this, if the incoming
//|         # signal stops, the last reading will remain
//|         # as the value.
//|         frequency.clear()
//|
STATIC mp_obj_t pulseio_frequencyin_make_new(const mp_obj_type_t *type, mp_uint_t n_args,
         mp_uint_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    pulseio_frequencyin_obj_t *self = m_new_obj(pulseio_frequencyin_obj_t);
    self->base.type = &pulseio_frequencyin_type;

    assert_pin(args[0], false);
    mcu_pin_obj_t *pin = MP_OBJ_TO_PTR(args[0]);
    assert_pin_free(pin);

    common_hal_pulseio_frequencyin_construct(self, pin);
    
    return MP_OBJ_FROM_PTR(self);
}

//|   .. method:: deinit()
//|
//|      Deinitialises the FrequencyIn and releases any hardware resources for reuse.
//|
STATIC mp_obj_t pulseio_frequencyin_deinit(mp_obj_t self_in) {
    pulseio_frequencyin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_pulseio_frequencyin_deinit(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pulseio_frequencyin_deinit_obj, pulseio_frequencyin_deinit);

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
STATIC mp_obj_t pulseio_frequencyin_obj___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    common_hal_pulseio_frequencyin_deinit(args[0]);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pulseio_frequencyin___exit___obj, 4, 4, pulseio_frequencyin_obj___exit__);

//|   .. method:: pause()
//|
//|     Pause frequency capture.
//|
STATIC mp_obj_t pulseio_frequencyin_obj_pause(mp_obj_t self_in) {
    pulseio_frequencyin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_frequencyin_deinited(self));

    common_hal_pulseio_frequencyin_pause(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(pulseio_frequencyin_pause_obj, pulseio_frequencyin_obj_pause);

//|   .. method:: resume()
//|
//|     Resumes frequency capture.
//|
STATIC mp_obj_t pulseio_frequencyin_obj_resume(mp_obj_t self_in) {
    pulseio_frequencyin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_frequencyin_deinited(self));
    
    common_hal_pulseio_frequencyin_resume(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(pulseio_frequencyin_resume_obj, pulseio_frequencyin_obj_resume);

//|   .. method:: clear()
//|
//|     Clears the last detected frequency capture value.
//|
STATIC mp_obj_t pulseio_frequencyin_obj_clear(mp_obj_t self_in) {
    pulseio_frequencyin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_frequencyin_deinited(self));
    
    common_hal_pulseio_frequencyin_clear(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(pulseio_frequencyin_clear_obj, pulseio_frequencyin_obj_clear);

//|   .. method:: __get__(index)
//|
//|     Returns the value of the last frequency captured.
//|
STATIC mp_obj_t pulseio_frequencyin_obj_get_value(mp_obj_t self_in) {
    pulseio_frequencyin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    raise_error_if_deinited(common_hal_pulseio_frequencyin_deinited(self));

    return MP_OBJ_NEW_SMALL_INT(common_hal_pulseio_frequencyin_get_item(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(pulseio_frequencyin_get_value_obj, pulseio_frequencyin_obj_get_value);

const mp_obj_property_t pulseio_frequencyin_value_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&pulseio_frequencyin_get_value_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

STATIC const mp_rom_map_elem_t pulseio_frequencyin_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pulseio_frequencyin_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&default___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&pulseio_frequencyin___exit___obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&pulseio_frequencyin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_pause), MP_ROM_PTR(&pulseio_frequencyin_pause_obj) },
    { MP_ROM_QSTR(MP_QSTR_resume), MP_ROM_PTR(&pulseio_frequencyin_resume_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&pulseio_frequencyin_clear_obj) },
};
STATIC MP_DEFINE_CONST_DICT(pulseio_frequencyin_locals_dict, pulseio_frequencyin_locals_dict_table);

const mp_obj_type_t pulseio_frequencyin_type = {
    { &mp_type_type },
    .name = MP_QSTR_frequencyin,
    .make_new = pulseio_frequencyin_make_new,
    .locals_dict = (mp_obj_dict_t*)&pulseio_frequencyin_locals_dict,
};
