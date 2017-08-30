/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Dan Halbert for Adafruit Industries
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

#include <math.h>
#include <stdint.h>

#include "py/objproperty.h"
#include "shared-bindings/processor/Processor.h"

#include "py/objproperty.h"
#include "py/runtime.h"

//| .. currentmodule:: processor
//|
//| :class:`Processor` --- Microcontroller CPU information and control
//| --------------------------------------------------------
//|
//| Get basic info about the microcontroller CPU and control it.
//|
//| You cannot create an instance of Processor, but you can access the sole instance available
//| by importing the microcontroller module and using its cpu attribute.
//|
//| Usage::
//|
//|    import microcontroller
//|    print(microcontroller.cpu.frequency)
//|    print(microcontroller.cpu.temperature)

//| .. class:: Processor()
//|
//| You cannot currently create an instance of `Processor`. Use `microcontroller.cpu` to access
//| the sole instance available.

STATIC mp_obj_t processor_cpu_make_new(const mp_obj_type_t *type,
         size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_raise_TypeError("cannot be created: use instance in microcontroller.cpu");
}

//| .. attribute:: frequency
//|
//|   Return the CPU operating frequency as an int, in Hz.
//|
STATIC mp_obj_t processor_cpu_get_frequency(mp_obj_t self) {
    return mp_obj_new_int_from_uint(common_hal_processor_cpu_get_frequency());
}

MP_DEFINE_CONST_FUN_OBJ_1(processor_cpu_get_frequency_obj, processor_cpu_get_frequency);

const mp_obj_property_t processor_cpu_frequency_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&processor_cpu_get_frequency_obj,  // getter
              (mp_obj_t)&mp_const_none_obj,            // no setter
              (mp_obj_t)&mp_const_none_obj,            // no deleter
    },
};

//| .. attribute:: temperature
//|
//|   Return the on-chip temperature, in Celsius, as a float.
//|   If the temperature is not available, return None.
//|
STATIC mp_obj_t processor_cpu_get_temperature(mp_obj_t self) {
    float temperature = common_hal_processor_cpu_get_temperature();
    return isnan(temperature) ? mp_const_none : mp_obj_new_float(temperature);
}

MP_DEFINE_CONST_FUN_OBJ_1(processor_cpu_get_temperature_obj, processor_cpu_get_temperature);

const mp_obj_property_t processor_cpu_temperature_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&processor_cpu_get_temperature_obj,  // getter
              (mp_obj_t)&mp_const_none_obj,            // no setter
              (mp_obj_t)&mp_const_none_obj,            // no deleter
    },
};

STATIC const mp_rom_map_elem_t processor_cpu_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_frequency), MP_ROM_PTR(&processor_cpu_frequency_obj) },
    { MP_ROM_QSTR(MP_QSTR_temperature), MP_ROM_PTR(&processor_cpu_temperature_obj) },
};

STATIC MP_DEFINE_CONST_DICT(processor_cpu_locals_dict, processor_cpu_locals_dict_table);

const mp_obj_type_t processor_cpu_type = {
    { &mp_type_type },
    .name = MP_QSTR_Processor,
    .make_new = processor_cpu_make_new,
    .locals_dict = (mp_obj_t)&processor_cpu_locals_dict,
};
