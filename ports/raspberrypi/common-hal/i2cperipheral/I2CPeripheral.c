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

#include "shared-bindings/i2cperipheral/I2CPeripheral.h"
#include "common-hal/busio/I2C.h"

#include "py/mperrno.h"
#include "py/runtime.h"

#include "shared-bindings/microcontroller/__init__.h"
#include "supervisor/shared/translate.h"

#include "src/rp2_common/hardware_gpio/include/hardware/gpio.h"


// Synopsys  DW_apb_i2c  (v2.01)  IP

#define NO_PIN 0xff
#define DEFAULT_ADDRESS 0x055

STATIC bool never_reset_i2c[2];
STATIC i2c_inst_t* i2c[2] = {i2c0, i2c1};

void common_hal_i2cperipheral_i2c_peripheral_construct(i2cperipheral_i2c_peripheral_obj_t *self,
        const mcu_pin_obj_t *scl, const mcu_pin_obj_t *sda,
        uint8_t *addresses, unsigned int num_addresses, bool smbus) {
    self->peripheral = NULL;
    // I2C pins have a regular pattern. SCL is always odd and SDA is even. They match up in pairs
    // so we can divide by two to get the instance. This pattern repeats.
    if (scl->number % 2 == 1 && sda->number % 2 == 0 && scl->number / 2 == sda->number / 2) {
        size_t instance = (scl->number / 2) % 2;
        self->peripheral = i2c[instance];
    }
    if (self->peripheral == NULL) {
        mp_raise_ValueError(translate("Invalid pins"));
    }
    if ((i2c_get_hw(self->peripheral)->enable & I2C_IC_ENABLE_ENABLE_BITS) != 0) {
        mp_raise_ValueError(translate("I2C peripheral in use"));
    }
//        if (frequency > 1000000) {
//        mp_raise_ValueError(translate("Unsupported baudrate"));
//    }

    gpio_set_function(sda->number, GPIO_FUNC_I2C);
    gpio_set_function(scl->number, GPIO_FUNC_I2C);

    i2c_set_slave_mode(self->peripheral, true, DEFAULT_ADDRESS);

    self->sda_pin = sda->number;
    self->scl_pin = scl->number;
    claim_pin(sda);
    claim_pin(scl);

}

bool common_hal_i2cperipheral_i2c_peripheral_deinited(i2cperipheral_i2c_peripheral_obj_t *self) {
    return self->sda_pin == NO_PIN;
}

void common_hal_i2cperipheral_i2c_peripheral_deinit(i2cperipheral_i2c_peripheral_obj_t *self) {
    if (common_hal_i2cperipheral_i2c_peripheral_deinited(self)) {
        return;
    }
    never_reset_i2c[i2c_hw_index(self->peripheral)] = false;

    i2c_deinit(self->peripheral);

    reset_pin_number(self->sda_pin);
    reset_pin_number(self->scl_pin);
    self->sda_pin = NO_PIN;
    self->scl_pin = NO_PIN;
}

void common_hal_i2cperipheral_i2c_peripheral_write(i2cperipheral_i2c_peripheral_obj_t *self,
                                   const uint8_t *data,
                                   size_t len) {
//    i2c_raw_blocking(self->peripheral, data, len);
}

int common_hal_i2cperipheral_i2c_peripheral_write_byte(i2cperipheral_i2c_peripheral_obj_t *self, uint8_t data) {

    return 0;
}

void common_hal_i2cperipheral_i2c_peripheral_ack(i2cperipheral_i2c_peripheral_obj_t *self, bool ack) {

}

void common_hal_i2cperipheral_i2c_peripheral_close(i2cperipheral_i2c_peripheral_obj_t *self) {

}

int common_hal_i2cperipheral_i2c_peripheral_read_byte(i2cperipheral_i2c_peripheral_obj_t *self, uint8_t *data) {

    return 0;
}

int common_hal_i2cperipheral_i2c_peripheral_is_addressed(i2cperipheral_i2c_peripheral_obj_t *self, uint8_t *address, bool *is_read, bool *is_restart)
{

    return 0;
}