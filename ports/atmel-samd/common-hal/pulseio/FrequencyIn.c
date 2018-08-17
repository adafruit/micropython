/*
 * This file is part of the MicroPython project, http://micropython.org/
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

#include "common-hal/pulseio/FrequencyIn.h"
#include "hal/include/hal_gpio.h"
#include "atmel_start_pins.h"

#include "mpconfigport.h"
#include "py/runtime.h"

#include "samd/clocks.h"
#include "samd/timers.h"
#include "samd/events.h"
#include "samd/pins.h"
#include "samd/external_interrupts.h"

#include "shared-bindings/microcontroller/__init__.h"
#include "shared-bindings/pulseio/FrequencyIn.h"
#include "peripheral_clk_config.h"
#include "hpl_gclk_config.h"

#ifdef SAMD21
#include "hpl/gclk/hpl_gclk_base.h"
#endif

static pulseio_frequencyin_obj_t *active_frequencyins[TC_INST_NUM];

// Current TC clocks are all 48MHz; lockup is occurring past a capture of 512kHz
// 48MHz / 512kHz = 93.75
static uint8_t MAX_FREQUENCY = 93;

void frequencyin_emergency_cancel_capture(uint8_t index) {
    pulseio_frequencyin_obj_t* self = active_frequencyins[index];
    
    NVIC_DisableIRQ(self->TC_IRQ);
    NVIC_ClearPendingIRQ(self->TC_IRQ);
    #ifdef SAMD21
    NVIC_DisableIRQ(EIC_IRQn);
    NVIC_ClearPendingIRQ(EIC_IRQn);
    #endif
    #ifdef SAMD51
    NVIC_DisableIRQ(EIC_0_IRQn + self->channel);
    NVIC_ClearPendingIRQ(EIC_0_IRQn + self->channel);
    #endif

    common_hal_pulseio_frequencyin_pause(self); // pause any further captures

    NVIC_EnableIRQ(self->TC_IRQ);
    #ifdef SAMD21
    NVIC_EnableIRQ(EIC_IRQn);
    #endif
    #ifdef SAMD51
    NVIC_EnableIRQ(EIC_0_IRQn + self->channel);
    #endif
    mp_raise_RuntimeError("Frequency captured is above capability. Capture Paused.");
}

void frequencyin_interrupt_handler(uint8_t index) {
    Tc* tc = tc_insts[index];

    if (!tc->COUNT16.INTFLAG.bit.MC0) return; // false trigger     

    pulseio_frequencyin_obj_t* self = active_frequencyins[index];

    // since we use EVACT.PPW, the Period is put into the CC[0] register
    // and that's all we need. CC[1] will contain the Pulse Width.
    uint16_t freq = tc->COUNT16.CC[0].bit.CC;
    self->frequency = freq;

    // we'll read CC[1] to make sure the INTFLAG.MCOx register is
    // also cleared. CC[1] will contain the Pulse Width measurement, which
    // we won't use.
    self->pulse_width = tc->COUNT16.CC[1].bit.CC;
    // SAMD51 Errata includes a CCBUFFx error that says to clear it twice
    // In Capture mode, CCBUFFx is cleared by reading CCx, so we read it one more time
    #ifdef SAMD51
    self->pulse_width = tc->COUNT16.CC[0].bit.CC;
    self->pulse_width = tc->COUNT16.CC[1].bit.CC;
    #endif

    // Check if we've reached the upper limit of detection (~512kHz)
    if (freq < MAX_FREQUENCY && freq > 0){
        frequencyin_emergency_cancel_capture(index);
    }
}

void common_hal_pulseio_frequencyin_construct(pulseio_frequencyin_obj_t* self, const mcu_pin_obj_t* pin) {

    if (!pin->has_extint) {
        mp_raise_RuntimeError("No hardware support on pin");
    }

    uint32_t mask = 1 << pin->extint_channel;
    if (eic_get_enable() == 1 &&
#ifdef SAMD21
    ((EIC->INTENSET.vec.EXTINT & mask) != 0 || (EIC->EVCTRL.vec.EXTINTEO & mask) != 0)) {
#endif
#ifdef SAMD51
    ((EIC->INTENSET.bit.EXTINT & mask) != 0 || (EIC->EVCTRL.bit.EXTINTEO & mask) != 0)) {
#endif
        mp_raise_RuntimeError("EXTINT channel already in use");
    }

    Tc *tc = NULL;
    int8_t index = TC_INST_NUM - 1;
    for (; index >= 0; index--) {
        if (tc_insts[index]->COUNT16.CTRLA.bit.ENABLE == 0) {
            tc = tc_insts[index];
            break;
        }
    }

    if (tc == NULL) {
        mp_raise_RuntimeError("All timers in use");
    }

    self->tc_index = index;
    self->pin = pin->pin;
    self->channel = pin->extint_channel;
    self->base_clock = 48000000;
    #ifdef SAMD21
    self->TC_IRQ = TC3_IRQn + index;
    #endif
    #ifdef SAMD51
    self->TC_IRQ = TC0_IRQn + index;
    #endif

    active_frequencyins[index] = self;

    // We use GCLK0 for SAMD21 and GCLK1 for SAMD51 because they both run at 48mhz making our
    // math the same across the boards.
    #ifdef SAMD21
    turn_on_clocks(true, index, 0);
    #endif
    #ifdef SAMD51
    turn_on_clocks(true, index, 1);
    #endif

    // Ensure EIC is on
    if (eic_get_enable() == 0) {
        turn_on_external_interrupt_controller(); // enables EIC, so disable it after
    }
    eic_set_enable(false);

    uint8_t sense_setting = EIC_CONFIG_SENSE0_HIGH_Val; //EIC_CONFIG_FILTEN0 | EIC_CONFIG_SENSE0_HIGH_Val;
    uint8_t config_index = self->channel / 8;
    uint8_t position = (self->channel % 8) * 4;
    uint32_t masked_value = EIC->CONFIG[config_index].reg & ~(0xf << position);
    EIC->CONFIG[config_index].reg = masked_value | (sense_setting << position);

    #ifdef SAMD21
    masked_value = EIC->EVCTRL.vec.EXTINTEO;
    EIC->EVCTRL.vec.EXTINTEO = masked_value | (1 << self->channel);
    #endif
    #ifdef SAMD51
    masked_value = EIC->EVCTRL.bit.EXTINTEO;
    EIC->EVCTRL.bit.EXTINTEO = masked_value | (1 << self->channel);
    EIC->ASYNCH.bit.ASYNCH = 1;
    #endif

    turn_on_cpu_interrupt(self->channel);

    eic_set_enable(true);

    #ifdef SAMD21
    tc_set_enable(tc, false);
    tc_reset(tc);
    tc->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 |
                            TC_CTRLA_PRESCALER_DIV1 |
                            TC_CTRLA_WAVEGEN_NFRQ;

    // Setup TC capture registers.
    tc->COUNT16.EVCTRL.bit.TCEI = 1;
    tc->COUNT16.EVCTRL.bit.EVACT = TC_EVCTRL_EVACT_PPW_Val;
    tc->COUNT16.CTRLC.bit.CPTEN0 = 1;
    tc->COUNT16.INTENSET.bit.MC0 = 1;
    #endif

    #ifdef SAMD51
    tc_set_enable(tc, false);
    tc_reset(tc);

    // setup capture registers.
    tc->COUNT16.EVCTRL.reg = TC_EVCTRL_EVACT(TC_EVCTRL_EVACT_PPW_Val) | TC_EVCTRL_TCEI;
    tc->COUNT16.WAVE.reg = TC_WAVE_WAVEGEN_NFRQ;
    tc->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 |
                            TC_CTRLA_PRESCALER_DIV1 |
                            (1 << TC_CTRLA_CAPTEN0_Pos);
    tc->COUNT16.INTENSET.bit.MC0 = 1;
    #endif

    NVIC_EnableIRQ(self->TC_IRQ);
    
    // Turn on EVSYS
    turn_on_event_system();
    uint8_t evsys_channel = find_async_event_channel();
    #ifdef SAMD21
    connect_event_user_to_channel((EVSYS_ID_USER_TC3_EVU + index), evsys_channel);
    #endif
    #ifdef SAMD51
    connect_event_user_to_channel((EVSYS_ID_USER_TC0_EVU + index), evsys_channel);
    #endif
    init_async_event_channel(evsys_channel, (EVSYS_ID_GEN_EIC_EXTINT_0 + self->channel));
    self->event_channel = evsys_channel;

    gpio_set_pin_function(pin->pin, GPIO_PIN_FUNCTION_A);

    tc_set_enable(tc, true);
    #ifdef SAMD51
    // setup the FREQM peripheral since DFLL is in open loop and may not be running
    // at exactly 48MHz. FREQM uses a free GCLK sourced from OSCULP32K (@ 32kHz) for the
    // reference clock, and GCLK1 as the DFLL clock to measure.
    freqm_init(GCLK_PCHCTRL_GEN_GCLK1_Val, 2);
    #endif
}

bool common_hal_pulseio_frequencyin_deinited(pulseio_frequencyin_obj_t* self) {
    return self->pin == NO_PIN;
}

void common_hal_pulseio_frequencyin_deinit(pulseio_frequencyin_obj_t* self) {
    if (common_hal_pulseio_frequencyin_deinited(self)) {
        return;
    }
    reset_pin(self->pin);

    // turn off EIC & EVSYS utilized by this TC
    disable_event_channel(self->event_channel);
    #ifdef SAMD21
    disable_event_user(EVSYS_ID_USER_TC3_EVU + self->tc_index);
    uint32_t masked_value = EIC->EVCTRL.vec.EXTINTEO;
    EIC->EVCTRL.vec.EXTINTEO = masked_value | (0 << self->channel);
    #endif
    #ifdef SAMD51
    disable_event_user(EVSYS_ID_USER_TC0_EVU + self->tc_index);
    uint32_t masked_value = EIC->EVCTRL.bit.EXTINTEO;
    EIC->EVCTRL.bit.EXTINTEO = masked_value | (0 << self->channel);
    EIC->ASYNCH.bit.ASYNCH = 0;
    freqm_deinit();
    #endif

    // check if any other objects are using the EIC; if not, turn it off
    if (EIC->EVCTRL.reg == 0 && EIC->INTENSET.reg == 0) {
        eic_set_enable(false);
        #ifdef SAMD21
        PM->APBAMASK.bit.EIC_ = false;
        hri_gclk_write_CLKCTRL_reg(GCLK, GCLK_CLKCTRL_ID(EIC_GCLK_ID));
        NVIC_DisableIRQ(EIC_IRQn);
        NVIC_ClearPendingIRQ(EIC_IRQn);
        #endif
    }

    // turn off the TC we were using
    Tc *tc = tc_insts[self->tc_index];
    tc_set_enable(tc, false);
    tc_reset(tc);
    NVIC_DisableIRQ(self->TC_IRQ);
    NVIC_ClearPendingIRQ(self->TC_IRQ);

    active_frequencyins[self->tc_index] = NULL;
    self->tc_index = 0xff;
    self->pin = NO_PIN;
    self->TC_IRQ = 0;

}

uint32_t common_hal_pulseio_frequencyin_get_item(pulseio_frequencyin_obj_t* self) {
    NVIC_DisableIRQ(self->TC_IRQ);
    #ifdef SAMD21
    NVIC_DisableIRQ(EIC_IRQn);
    #endif
    #ifdef SAMD51
    NVIC_DisableIRQ(EIC_0_IRQn + self->channel);

    // get the current clock speed of DFLL48MHz using FREQM
    self->base_clock = freqm_read();
    if (self->base_clock < 0) {
        mp_raise_RuntimeError("An error occurred in measuring the frequency. (FREQM peripheral usage.)");
    }
    #endif

    uint32_t value = self->base_clock / self->frequency;

    NVIC_ClearPendingIRQ(self->TC_IRQ);
    NVIC_EnableIRQ(self->TC_IRQ);
    #ifdef SAMD21
    NVIC_ClearPendingIRQ(EIC_IRQn);
    NVIC_EnableIRQ(EIC_IRQn);
    #endif
    #ifdef SAMD51
    NVIC_ClearPendingIRQ(EIC_0_IRQn + self->channel);
    NVIC_EnableIRQ(EIC_0_IRQn + self->channel);
    #endif

    return value;
}

void common_hal_pulseio_frequencyin_pause(pulseio_frequencyin_obj_t* self) {
    Tc *tc = tc_insts[self->tc_index];
    if (!tc->COUNT16.EVCTRL.bit.TCEI) {
        return;
    }
    tc->COUNT16.EVCTRL.bit.TCEI = 0;
    
    #ifdef SAMD21
    uint32_t masked_value = EIC->EVCTRL.vec.EXTINTEO;
    EIC->EVCTRL.vec.EXTINTEO = masked_value | (0 << self->channel);
    #endif
    #ifdef SAMD51
    uint32_t masked_value = EIC->EVCTRL.bit.EXTINTEO;
    EIC->EVCTRL.bit.EXTINTEO = masked_value | (0 << self->channel);
    #endif
    return;
}

void common_hal_pulseio_frequencyin_resume(pulseio_frequencyin_obj_t* self) {
    Tc *tc = tc_insts[self->tc_index];
    if (tc->COUNT16.EVCTRL.bit.TCEI) {
        return;
    }
    tc->COUNT16.EVCTRL.bit.TCEI = 1;
    
    #ifdef SAMD21
    uint32_t masked_value = EIC->EVCTRL.vec.EXTINTEO;
    EIC->EVCTRL.vec.EXTINTEO = masked_value | (1 << self->channel);
    #endif
    #ifdef SAMD51
    uint32_t masked_value = EIC->EVCTRL.bit.EXTINTEO;
    EIC->EVCTRL.bit.EXTINTEO = masked_value | (1 << self->channel);
    #endif
    return;
}

void common_hal_pulseio_frequencyin_clear(pulseio_frequencyin_obj_t* self) {
    NVIC_DisableIRQ(self->TC_IRQ);
    #ifdef SAMD21
    NVIC_DisableIRQ(EIC_IRQn);
    #endif
    #ifdef SAMD51
    NVIC_DisableIRQ(EIC_0_IRQn + self->channel);
    #endif

    self->frequency = 0;
    self->pulse_width = 0;

    NVIC_ClearPendingIRQ(self->TC_IRQ);
    NVIC_EnableIRQ(self->TC_IRQ);
    #ifdef SAMD21
    NVIC_ClearPendingIRQ(EIC_IRQn);
    NVIC_EnableIRQ(EIC_IRQn);
    #endif
    #ifdef SAMD51
    NVIC_ClearPendingIRQ(EIC_0_IRQn + self->channel);
    NVIC_EnableIRQ(EIC_0_IRQn + self->channel);
    #endif
}
