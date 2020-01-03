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

#include "py/objproperty.h"
#include "lib/utils/buffer_helper.h"
#include "py/runtime.h"

#include "shared-bindings/usb_hid/Device.h"


//| .. currentmodule:: usb_hid
//|
//| :class:`Device` -- HID Device
//| ============================================
//|
//| Usage::
//|
//|    import usb_hid
//|
//|    mouse = usb_hid.devices[0]
//|
//|    mouse.send_report()
//|

//| .. class:: Device()
//|
//|   Not currently dynamically supported.
//|
//|   .. method:: send_report(buf)
//|
//|     Send a HID report.
//|
STATIC mp_obj_t usb_hid_device_send_report(mp_obj_t self_in, mp_obj_t buffer) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buffer, &bufinfo, MP_BUFFER_READ);

    common_hal_usb_hid_device_send_report(self, ((uint8_t*) bufinfo.buf), bufinfo.len);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(usb_hid_device_send_report_obj, usb_hid_device_send_report);

//|   .. attribute:: usage_page
//|
//|     The usage page of the device as an `int`. Can be thought of a category. (read-only)
//|
STATIC mp_obj_t usb_hid_device_obj_get_usage_page(mp_obj_t self_in) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_hid_device_get_usage_page(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_hid_device_get_usage_page_obj, usb_hid_device_obj_get_usage_page);

const mp_obj_property_t usb_hid_device_usage_page_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&usb_hid_device_get_usage_page_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

//|   .. attribute:: usage
//|
//|     The functionality of the device as an int. (read-only)
//|
//|     For example, Keyboard is 0x06 within the generic desktop usage page 0x01.
//|     Mouse is 0x02 within the same usage page.
//|
STATIC mp_obj_t usb_hid_device_obj_get_usage(mp_obj_t self_in) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_hid_device_get_usage(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_hid_device_get_usage_obj,
                          usb_hid_device_obj_get_usage);

const mp_obj_property_t usb_hid_device_usage_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&usb_hid_device_get_usage_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

//|   .. method:: read_report_into(self, buffer, start=0, end=None)
//|
//|     Read the next out-report from the queue into a buffer.
//|     Return the number of bytes read.
//|
STATIC mp_obj_t usb_hid_device_read_report_into(
        size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_buffer, ARG_start, ARG_end };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buffer,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_start,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_end,        MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = INT_MAX} },
    };
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_WRITE);
    size_t length = bufinfo.len;
    int32_t start = args[ARG_start].u_int;
    int32_t end = args[ARG_end].u_int;
    uint8_t *buffer = bufinfo.buf;
    normalize_buffer_bounds(&start, end, &length);
    if (length == 0) {
        mp_raise_ValueError(translate("Buffer must be at least length 1"));
    }
    size_t count = self->out_report_count;
    if (self->out_report_count) {
        self->out_report_count = 0;
        buffer[start] = self->out_report_buffer[0];
    }
    return mp_obj_new_int(count);
}
MP_DEFINE_CONST_FUN_OBJ_KW(usb_hid_device_read_report_into_obj, 2,
                           usb_hid_device_read_report_into);

STATIC const mp_rom_map_elem_t usb_hid_device_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_send_report),    MP_ROM_PTR(&usb_hid_device_send_report_obj) },
    { MP_ROM_QSTR(MP_QSTR_usage_page),     MP_ROM_PTR(&usb_hid_device_usage_page_obj)},
    { MP_ROM_QSTR(MP_QSTR_usage),          MP_ROM_PTR(&usb_hid_device_usage_obj)},
    { MP_ROM_QSTR(MP_QSTR_read_report_into),          MP_ROM_PTR(&usb_hid_device_read_report_into_obj)},
};

STATIC MP_DEFINE_CONST_DICT(usb_hid_device_locals_dict, usb_hid_device_locals_dict_table);

const mp_obj_type_t usb_hid_device_type = {
    { &mp_type_type },
    .name = MP_QSTR_Device,
    .locals_dict = (mp_obj_t)&usb_hid_device_locals_dict,
};
