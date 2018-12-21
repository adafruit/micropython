/*
 * This file is part of the MicroPython project, http://micropython.org/
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

#include "common-hal/pulseio/Counter.h"

#include <stdint.h>

#include "atmel_start_pins.h"
#include "hal/include/hal_gpio.h"

#include "background.h"
#include "mpconfigport.h"
#include "py/gc.h"
#include "py/runtime.h"
#include "samd/external_interrupts.h"
#include "samd/pins.h"
#include "shared-bindings/microcontroller/__init__.h"
#include "shared-bindings/pulseio/Counter.h"
#include "supervisor/shared/translate.h"

static void counter_interrupt_handler(uint8_t channel) {
    pulseio_counter_obj_t* self = get_eic_channel_data(channel);
    self->count++;
}

void common_hal_pulseio_counter_construct(pulseio_counter_obj_t* self, const mcu_pin_obj_t* pin) {
    if (!pin->has_extint) {
        mp_raise_RuntimeError(translate("No hardware support on pin"));
    }
    if (eic_get_enable() && !eic_channel_free(pin->extint_channel)) {
        mp_raise_RuntimeError(translate("EXTINT channel already in use"));
    }

    self->channel = pin->extint_channel;
    self->pin = pin->number;
    self->count = 0;

    set_eic_channel_handler(pin->extint_channel, counter_interrupt_handler);
    set_eic_channel_data(pin->extint_channel, (void*) self);

    // Check to see if the EIC is enabled and start it up if its not.'
    if (eic_get_enable() == 0) {
        turn_on_external_interrupt_controller();
    }

    gpio_set_pin_function(pin->number, GPIO_PIN_FUNCTION_A);

    // TODO: Make this configurable
    enum gpio_pull_mode asf_pull = GPIO_PULL_UP;
    gpio_set_pin_pull_mode(pin->number, asf_pull);

    turn_on_cpu_interrupt(self->channel);

    claim_pin(pin);

    // TODO: Make this configurable
    uint32_t sense_setting;
    sense_setting = EIC_CONFIG_SENSE0_BOTH_Val;
//  sense_setting = EIC_CONFIG_SENSE0_FALL_Val;
//  sense_setting = EIC_CONFIG_SENSE0_RISE_Val;

    turn_on_eic_channel(self->channel, sense_setting, EIC_HANDLER_FUNC);
}

bool common_hal_pulseio_counter_deinited(pulseio_counter_obj_t* self) {
    return self->pin == NO_PIN;
}

void common_hal_pulseio_counter_deinit(pulseio_counter_obj_t* self) {
    if (common_hal_pulseio_counter_deinited(self)) {
        return;
    }
    turn_off_eic_channel(self->channel);
    reset_pin_number(self->pin);
    self->pin = NO_PIN;
}

uint32_t common_hal_pulseio_counter_get_count(pulseio_counter_obj_t* self, bool clear) {
    common_hal_mcu_disable_interrupts();
    uint32_t count = self->count;
    if (clear) {
        self->count = 0;
    }
    common_hal_mcu_enable_interrupts();
    return count;
}

bool common_hal_pulseio_counter_get_pinvalue(pulseio_counter_obj_t* self) {
    return gpio_get_pin_level(self->pin);
}
