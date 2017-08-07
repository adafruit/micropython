/*
 * This file is part of the Micro Python project, http://micropython.org/
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

#include "py/binary.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/nvm/ByteArray.h"

//| .. currentmodule:: nvm
//|
//| :class:`ByteArray` -- Presents a stretch of non-volatile memory as a bytearray.
//| ================================================================================
//|
//| Non-volatile memory is avialble as a byte array that persists over reloads
//| and power cycles.
//|
//| Usage::
//|
//|    import microcontroller
//|    microcontroller.nvm[0] = 0xcc
//|

//| .. class:: ByteArray()
//|
//|   Not currently dynamically supported. Access one through `microcontroller.nvm`.
//|
STATIC mp_obj_t nvm_bytearray_make_new(const mp_obj_type_t *type,
        mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    return mp_const_none;
}

//|   .. method:: __len__()
//|
//|     Return the length. This is used by (`len`)
//|
STATIC mp_obj_t nvm_bytearray___len__(mp_obj_t self_in) {
    nvm_bytearray_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return MP_OBJ_NEW_SMALL_INT(common_hal_nvm_bytearray_get_length(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(nvm_bytearray___len___obj, nvm_bytearray___len__);

STATIC const mp_rom_map_elem_t nvm_bytearray_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___len__),    MP_ROM_PTR(&nvm_bytearray___len___obj) },
};

STATIC MP_DEFINE_CONST_DICT(nvm_bytearray_locals_dict, nvm_bytearray_locals_dict_table);

STATIC mp_obj_t nvm_bytearray_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    if (value == MP_OBJ_NULL) {
        // delete item
        // slice deletion
        return MP_OBJ_NULL; // op not supported
    } else {
        nvm_bytearray_obj_t *self = MP_OBJ_TO_PTR(self_in);
        if (0) {
#if MICROPY_PY_BUILTINS_SLICE
        } else if (MP_OBJ_IS_TYPE(index_in, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(common_hal_nvm_bytearray_get_length(self), index_in, &slice)) {
                mp_raise_NotImplementedError("only slices with step=1 (aka None) are supported");
            }
            if (value != MP_OBJ_SENTINEL) {
                #if MICROPY_PY_ARRAY_SLICE_ASSIGN
                // Assign
                size_t src_len = slice.stop - slice.start;
                uint8_t* src_items;
                if (MP_OBJ_IS_TYPE(value, &mp_type_array) ||
                        MP_OBJ_IS_TYPE(value, &mp_type_bytearray) ||
                        MP_OBJ_IS_TYPE(value, &mp_type_memoryview) ||
                        MP_OBJ_IS_TYPE(value, &mp_type_bytes)) {
                    mp_buffer_info_t bufinfo;
                    mp_get_buffer_raise(value, &bufinfo, MP_BUFFER_READ);
                    if (bufinfo.len != src_len) {
                        mp_raise_ValueError("Slice and value different lengths.");
                    }
                    src_len = bufinfo.len;
                    src_items = bufinfo.buf;
                    if (1 != mp_binary_get_size('@', bufinfo.typecode, NULL)) {
                        mp_raise_ValueError("Array values should be single bytes.");
                    }
                } else {
                    mp_raise_NotImplementedError("array/bytes required on right side");
                }

                if (!common_hal_nvm_bytearray_set_bytes(self, slice.start, src_items, src_len)) {
                    mp_raise_RuntimeError("Unable to write to nvm.");
                }
                return mp_const_none;
                #else
                return MP_OBJ_NULL; // op not supported
                #endif
            } else {
                // Read slice.
                size_t len = slice.stop - slice.start;
                uint8_t *items = m_new(uint8_t, len);
                common_hal_nvm_bytearray_get_bytes(self, slice.start, len, items);
                return mp_obj_new_bytearray_by_ref(len, items);
            }
#endif
        } else {
            // Single index rather than slice.
            size_t index = mp_get_index(self->base.type, self->len, index_in, false);
            if (value == MP_OBJ_SENTINEL) {
                // load
                uint8_t value_out;
                common_hal_nvm_bytearray_get_bytes(self, index, 1, &value_out);
                return MP_OBJ_NEW_SMALL_INT(value_out);
            } else {
                // store
                mp_int_t byte_value = mp_obj_get_int(value);
                if (byte_value > 0xff || byte_value < 0) {
                    mp_raise_ValueError("Bytes must be between 0 and 255.");
                }
                uint8_t short_value = byte_value;
                if (!common_hal_nvm_bytearray_set_bytes(self, index, &short_value, 1)) {
                    mp_raise_RuntimeError("Unable to write to nvm.");
                }
                return mp_const_none;
            }
        }
    }
}

const mp_obj_type_t nvm_bytearray_type = {
    { &mp_type_type },
    .name = MP_QSTR_ByteArray,
    .make_new = nvm_bytearray_make_new,
    .subscr = nvm_bytearray_subscr,
    .print = NULL,
    .locals_dict = (mp_obj_t)&nvm_bytearray_locals_dict,
};
