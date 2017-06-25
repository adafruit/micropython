/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries, Dan Halbert
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

#include "py/obj.h"
#include "py/runtime.h"

#include "shared-bindings/umem/__init__.h"

//| :mod:`umem` --- Memory use analysis
//| ========================================================
//|
//| .. module:: umem
//|   :synopsis: system-related functions
//|

//| .. method:: info(object)
//|
//|   Prints memory debugging info for the given object and returns the
//|   estimated size.
//|
STATIC mp_obj_t umem_info(mp_obj_t obj) {
    uint32_t size = shared_module_umem_info(obj);

    return MP_OBJ_NEW_SMALL_INT(size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(umem_info_obj, umem_info);

#if MICROPY_PY_UMEM_MAX_STACK_USE
//| .. method:: max_stack_use()
//|
//|   Return the maximum excursion of the stack so far.
//|   Use in conjunction with stack_size() and micropython.stack_use()
//|   to monitor stack memory use.
//|
//|   Use with umem.stack_size()
//|   and micropython.stack_use() to monitor stack memory usage.
//|
STATIC mp_obj_t max_stack_use(void) {
    return MP_OBJ_NEW_SMALL_INT(shared_module_umem_max_stack_use());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(max_stack_use_obj, max_stack_use);

#endif // MICROPY_PY_UMEM_MAX_STACK_USE

//| .. method:: stack_size()
//|
//|   Return the size of the entire stack.
//|   Same as in micropython.mem_info(), but returns a value instead
//|   of just printing it.
//|
//|   Use with umem.max_stack_use() (if available)
//|   and micropython.stack_use() to monitor stack memory usage.
//|
STATIC mp_obj_t stack_size(void) {
    return MP_OBJ_NEW_SMALL_INT(shared_module_umem_stack_size());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(stack_size_obj, stack_size);

STATIC const mp_rom_map_elem_t umem_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_umem) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&umem_info_obj) },
    #if MICROPY_PY_UMEM_MAX_STACK_USE
    { MP_ROM_QSTR(MP_QSTR_max_stack_use), MP_ROM_PTR(&max_stack_use_obj) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_stack_size), MP_ROM_PTR(&stack_size_obj) },
};

STATIC MP_DEFINE_CONST_DICT(umem_module_globals, umem_module_globals_table);

const mp_obj_module_t umem_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&umem_module_globals,
};
