/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries
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

#include "py/obj.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "shared-bindings/processor/__init__.h"
#include "shared-bindings/processor/Processor.h"

//| :mod:`processor` --- Microcontroller CPU information and control
//| ================================================================
//|
//| .. module:: processor
//|   :synopsis: CPU information and control
//|   :platform: SAMD21,ESP8266
//|
//| The `processor` module defines the class `Processor`.
//| It provides microcontroller CPU information and control, such as
//| temperature and clock frequency.
//|
//| There is only one instance of Processor, available in `microcontroller.cpu`.
//|

//| Libraries
//|
//| .. toctree::
//|     :maxdepth: 3
//|
//|     Processor
STATIC const mp_rom_map_elem_t processor_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_processor) },
    { MP_ROM_QSTR(MP_QSTR_Processor),   MP_ROM_PTR(&processor_cpu_type) },
};

STATIC MP_DEFINE_CONST_DICT(processor_module_globals, processor_module_globals_table);

const mp_obj_module_t processor_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&processor_module_globals,
};
