/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Jose David Montoya
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

#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/i2cperipheral/I2CPeripheral.h"
#include "shared-bindings/time/__init__.h"
#include "shared-bindings/util.h"

#include "lib/utils/buffer_helper.h"
#include "lib/utils/context_manager_helpers.h"
#include "lib/utils/interrupt_char.h"

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objproperty.h"
#include "py/runtime.h"


STATIC mp_obj_t mp_obj_new_i2cperipheral_i2c_peripheral_request(i2cperipheral_i2c_peripheral_obj_t *peripheral) {
    i2cperipheral_i2c_peripheral_request_obj_t *self = m_new_obj(i2cperipheral_i2c_peripheral_request_obj_t);
    self->peripheral = peripheral;
    return (mp_obj_t)self;
}

//| class I2CPeripheral:
//|     """Two wire serial protocol peripheral"""
//|
//|     def __init__(self, scl: microcontroller.Pin, sda: microcontroller.Pin) -> None:
//|         """I2C is a two-wire protocol for communicating between devices.
//|         This implements the peripheral (sensor, secondary) side.
//|
//|         :param ~microcontroller.Pin scl: The clock pin
//|         :param ~microcontroller.Pin sda: The data pin"""
//|         ...
//|

STATIC mp_obj_t i2cperipheral_i2c_peripheral_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    i2cperipheral_i2c_peripheral_obj_t *self = m_new_obj(i2cperipheral_i2c_peripheral_obj_t);
    // self->base.type = &i2cperipheral_i2c_peripheral_obj_t;
    enum {ARG_scl, ARG_sda};
    static const mp_arg_t allowed_args[] = {
    { MP_QSTR_scl, MP_ARG_REQUIRED | MP_ARG_OBJ },
    { MP_QSTR_sda, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    const mcu_pin_obj_t* scl = validate_obj_is_free_pin(args[ARG_scl].u_obj);
    const mcu_pin_obj_t* sda = validate_obj_is_free_pin(args[ARG_sda].u_obj);

    uint8_t address = 0x055;

    //TODO verify if adress given is not out of bounds
    common_hal_i2cperipheral_i2c_peripheral_construct(self, scl, sda);
    return (mp_obj_t)self;
    }

//|     def deinit(self) -> None:
//|         """Releases control of the underlying hardware so other classes can use it."""
//|         ...
//|
STATIC mp_obj_t i2cperipheral_i2c_peripheral_obj_deinit(mp_obj_t self_in) {
    mp_check_self(MP_OBJ_IS_TYPE(self_in, &i2cperipheral_i2c_peripheral_obj_t));
    i2cperipheral_i2c_peripheral_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_i2cperipheral_i2c_peripheral_deinit(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(i2cperipheral_i2c_peripheral_deinit_obj, i2cperipheral_i2c_peripheral_obj_deinit);

//|     def __enter__(self) -> I2CPeripheral:
//|         """No-op used in Context Managers."""
//|         ...
//|
//  Provided by context manager helper.

//|     def __exit__(self) -> None:
//|         """Automatically deinitializes the hardware on context exit. See
//|         :ref:`lifetime-and-contextmanagers` for more info."""
//|         ...
//|
STATIC mp_obj_t i2cperipheral_i2c_peripheral_obj___exit__(size_t n_args, const mp_obj_t *args) {
    mp_check_self(MP_OBJ_IS_TYPE(args[0], &i2cperipheral_i2c_peripheral_obj_t));
    i2cperipheral_i2c_peripheral_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    common_hal_i2cperipheral_i2c_peripheral_deinit(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(i2cperipheral_i2c_peripheral___exit___obj, 4, 4, i2cperipheral_i2c_peripheral_obj___exit__);



//|     def writeto(self, address: int, buffer: ReadableBuffer, *, start: int = 0, end: Optional[int] = None, stop: bool = True) -> None:
//|         """Write the bytes from ``buffer`` to the device selected by ``address`` and
//|         then transmit a stop bit.
//|
//|         If ``start`` or ``end`` is provided, then the buffer will be sliced
//|         as if ``buffer[start:end]``. This will not cause an allocation like
//|         ``buffer[start:end]`` will so it saves memory.
//|
//|         Writing a buffer or slice of length zero is permitted, as it can be used
//|         to poll for the existence of a device.
//|
//|         :param int address: 7-bit device address
//|         :param ~_typing.ReadbleBuffer buffer: buffer containing the bytes to write
//|         :param int start: Index to start writing from
//|         :param int end: Index to read up to but not include. Defaults to ``len(buffer)``"""
//|         ...
//|
// Shared arg parsing for writeto and writeto_then_readfrom.
// STATIC void writeto(i2cperipheral_i2c_obj_t *self, mp_obj_t buffer, int32_t start, mp_int_t end) {
//     // get the buffer to write the data from
//     mp_buffer_info_t bufinfo;
//     mp_get_buffer_raise(buffer, &bufinfo, MP_BUFFER_READ);

//     size_t length = bufinfo.len;
//     normalize_buffer_bounds(&start, end, &length);

//     // do the transfer
//     uint8_t status = common_hal_pheripheral_i2c_write(self, ((uint8_t*) bufinfo.buf) + start,
//                                                 length, stop);
//     if (status != 0) {
//         mp_raise_OSError(status);
//     }
// }

// STATIC mp_obj_t i2cperipheral_i2c_peripheral_write(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
//     enum { ARG_buffer, ARG_start, ARG_lenght,  ARG_end };
//     static const mp_arg_t allowed_args[] = {
//         { MP_QSTR_buffer,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
//         { MP_QSTR_lenght,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
//         { MP_QSTR_start,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
//         { MP_QSTR_end,        MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = INT_MAX} },
//     };
//     i2cpheripheral_i2c_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
//     check_for_deinit(self);
//     check_lock(self);
//     mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
//     mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

//     writeto(self, args[ARG_buffer].u_obj, args[ARG_start].u_int,
//             args[ARG_lenght].u_int, args[ARG_end].u_int);
//     return mp_const_none;
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_KW(busio_i2c_writeto_obj, 1, busio_i2c_writeto);