/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Lucian Copeland
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

// This file contains all of the Python API definitions for the
// spiperipheral.SPIPeripheral class.

#include <string.h>

#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/spiperipheral/SPIPeripheral.h"
#include "shared-bindings/util.h"

#include "lib/utils/buffer_helper.h"
#include "lib/utils/context_manager_helpers.h"
#include "py/mperrno.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "supervisor/shared/translate.h"


//| class SPIPeripheral:

STATIC mp_obj_t spiperipheral_spi_peripheral_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    spiperipheral_spi_peripheral_obj_t *self = m_new_obj(spiperipheral_spi_peripheral_obj_t);
    self->base.type = &spiperipheral_spi_peripheral_type;
    enum { ARG_SCK, ARG_MOSI, ARG_MISO, ARG_CS };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_SCK, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_MOSI, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_MISO, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_CS, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    const mcu_pin_obj_t* sck = validate_obj_is_free_pin(args[ARG_SCK].u_obj);
    const mcu_pin_obj_t* mosi = validate_obj_is_free_pin_or_none(args[ARG_MOSI].u_obj);
    const mcu_pin_obj_t* miso = validate_obj_is_free_pin_or_none(args[ARG_MISO].u_obj);
    const mcu_pin_obj_t* cs = validate_obj_is_free_pin(args[ARG_CS].u_obj);

    if (!miso && !mosi) {
        mp_raise_ValueError(translate("Must provide MISO or MOSI pin"));
    }

    common_hal_spiperipheral_spi_peripheral_construct(self, sck, mosi, miso, cs);
    return MP_OBJ_FROM_PTR(self);
}

//|     def deinit(self) -> None:
//|         """Turn off the SPI Peripheral bus."""
//|         ...
//|
STATIC mp_obj_t spiperipheral_spi_peripheral_obj_deinit(mp_obj_t self_in) {
    spiperipheral_spi_peripheral_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_spiperipheral_spi_peripheral_deinit(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(spiperipheral_spi_peripheral_deinit_obj, spiperipheral_spi_peripheral_obj_deinit);

//|     def __enter__(self) -> SPI:
//|         """No-op used by Context Managers.
//|         Provided by context manager helper."""
//|         ...
//|

//|     def __exit__(self) -> None:
//|         """Automatically deinitializes the hardware when exiting a context. See
//|         :ref:`lifetime-and-contextmanagers` for more info."""
//|         ...
//|
STATIC mp_obj_t spiperipheral_spi_peripheral_obj___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    common_hal_spiperipheral_spi_peripheral_deinit(args[0]);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(spiperipheral_spi_peripheral_obj___exit___obj, 4, 4, spiperipheral_spi_peripheral_obj___exit__);

STATIC void check_for_deinit(spiperipheral_spi_peripheral_obj_t *self) {
    if (common_hal_spiperipheral_spi_peripheral_deinited(self)) {
        raise_deinited_error();
    }
}

STATIC mp_obj_t spiperipheral_spi_peripheral_obj_spi_ready(mp_obj_t self_in) {
    spiperipheral_spi_peripheral_obj_t *self = MP_OBJ_TO_PTR(self_in);
    check_for_deinit(self);
    return mp_obj_new_bool(common_hal_spiperipheral_spi_peripheral_spi_ready(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(spiperipheral_spi_peripheral_spi_ready_obj, spiperipheral_spi_peripheral_obj_spi_ready);

STATIC mp_obj_t spiperipheral_spi_peripheral_obj_transaction_complete(mp_obj_t self_in) {
    spiperipheral_spi_peripheral_obj_t *self = MP_OBJ_TO_PTR(self_in);
    check_for_deinit(self);
    return mp_obj_new_bool(common_hal_spiperipheral_spi_peripheral_transaction_complete(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(spiperipheral_spi_peripheral_transaction_complete_obj, spiperipheral_spi_peripheral_obj_transaction_complete);

STATIC mp_obj_t spiperipheral_spi_peripheral_obj_transaction_error(mp_obj_t self_in) {
    spiperipheral_spi_peripheral_obj_t *self = MP_OBJ_TO_PTR(self_in);
    check_for_deinit(self);
    return mp_obj_new_bool(common_hal_spiperipheral_spi_peripheral_transaction_error(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(spiperipheral_spi_peripheral_transaction_error_obj, spiperipheral_spi_peripheral_obj_transaction_error);

STATIC mp_obj_t spiperipheral_spi_peripheral_wait_for_transaction(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_buffer_out, ARG_buffer_in, ARG_length};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buffer_out,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_buffer_in,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_length,        MP_ARG_REQUIRED | MP_ARG_INT },
    };

    spiperipheral_spi_peripheral_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    check_for_deinit(self);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t buf_out_info;
    mp_get_buffer_raise(args[ARG_buffer_out].u_obj, &buf_out_info, MP_BUFFER_READ);
    mp_buffer_info_t buf_in_info;
    mp_get_buffer_raise(args[ARG_buffer_in].u_obj, &buf_in_info, MP_BUFFER_WRITE);

    if (args[ARG_length].u_int == 0) {
        return mp_const_none;
    }

    bool ok = common_hal_spiperipheral_spi_peripheral_wait_for_transaction(self,
        ((const uint8_t*)buf_out_info.buf), ((uint8_t*)buf_in_info.buf), args[ARG_length].u_int);

    if (!ok) {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(spiperipheral_spi_peripheral_wait_for_transaction_obj, 3, spiperipheral_spi_peripheral_wait_for_transaction);



STATIC const mp_rom_map_elem_t spiperipheral_spi_peripheral_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&spiperipheral_spi_peripheral_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&default___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&spiperipheral_spi_peripheral_obj___exit___obj) },

    { MP_ROM_QSTR(MP_QSTR_ready), MP_ROM_PTR(&spiperipheral_spi_peripheral_spi_ready_obj) },
    { MP_ROM_QSTR(MP_QSTR_complete), MP_ROM_PTR(&spiperipheral_spi_peripheral_transaction_complete_obj) },
    { MP_ROM_QSTR(MP_QSTR_error), MP_ROM_PTR(&spiperipheral_spi_peripheral_transaction_error_obj) },

    { MP_ROM_QSTR(MP_QSTR_wait_for_transaction), MP_ROM_PTR(&spiperipheral_spi_peripheral_wait_for_transaction_obj) },
};
STATIC MP_DEFINE_CONST_DICT(spiperipheral_spi_peripheral_locals_dict, spiperipheral_spi_peripheral_locals_dict_table);

const mp_obj_type_t spiperipheral_spi_peripheral_type = {
   { &mp_type_type },
   .name = MP_QSTR_SPIPERIPHERAL,
   .make_new = spiperipheral_spi_peripheral_make_new,
   .locals_dict = (mp_obj_dict_t*)&spiperipheral_spi_peripheral_locals_dict,
};
