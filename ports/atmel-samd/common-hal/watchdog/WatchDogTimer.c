/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Adafruit Industries
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

#include "py/runtime.h"

#include "common-hal/watchdog/WatchDogTimer.h"

#include "shared-bindings/watchdog/__init__.h"
#include "shared-bindings/microcontroller/__init__.h"

#include <sam.h>

STATIC bool _watchdog_samd_initialized;

STATIC void _watchdog_samd_reset(void) {
  // Write the watchdog clear key value (0xA5) to the watchdog
  // clear register to clear the watchdog timer and reset it.
#ifdef SAM_D5X_E5X
  while (WDT->SYNCBUSY.reg)
    ;
#else
  while (WDT->STATUS.bit.SYNCBUSY)
    ;
#endif
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
}

STATIC void _watchdog_samd_disable(void) {
#ifdef SAM_D5X_E5X
  WDT->CTRLA.bit.ENABLE = 0;
  while (WDT->SYNCBUSY.reg)
    ;
#else
  WDT->CTRL.bit.ENABLE = 0;
  while (WDT->STATUS.bit.SYNCBUSY)
    ;
#endif
}

STATIC void _watchdog_samd_initialize_wdt(void) {
  // One-time initialization of watchdog timer.
  // Insights from rickrlh and rbrucemtl in Arduino forum!

#ifdef SAM_D5X_E5X
  // SAMD51 WDT uses OSCULP32k as input clock now
  // section: 20.5.3
  OSC32KCTRL->OSCULP32K.bit.EN1K = 1;  // Enable out 1K (for WDT)
  OSC32KCTRL->OSCULP32K.bit.EN32K = 0; // Disable out 32K

  // Enable WDT early-warning interrupt
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0); // Top priority
  NVIC_EnableIRQ(WDT_IRQn);

  while (WDT->SYNCBUSY.reg)
    ;

  USB->DEVICE.CTRLA.bit.ENABLE = 0; // Disable the USB peripheral
  while (USB->DEVICE.SYNCBUSY.bit.ENABLE)
    ;                                 // Wait for synchronization
  USB->DEVICE.CTRLA.bit.RUNSTDBY = 0; // Deactivate run on standby
  USB->DEVICE.CTRLA.bit.ENABLE = 1;   // Enable the USB peripheral
  while (USB->DEVICE.SYNCBUSY.bit.ENABLE)
    ; // Wait for synchronization
#else
  // Generic clock generator 2, divisor = 32 (2^(DIV+1))
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
  // Enable clock generator 2 using low-power 32KHz oscillator.
  // With /32 divisor above, this yields 1024Hz(ish) clock.
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_GENEN |
                      GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;
  // WDT clock = clock gen 2
  GCLK->CLKCTRL.reg =
      GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2;

  // Enable WDT early-warning interrupt
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0); // Top priority
  NVIC_EnableIRQ(WDT_IRQn);
#endif

  _watchdog_samd_initialized = true;
}

STATIC int _watchdog_samd_enable(int maxPeriodMS, bool earlyWarning) {
  // Enable the watchdog with a period up to the specified max period in
  // milliseconds.

  // Review the watchdog section from the SAMD21 datasheet section 17:
  // http://www.atmel.com/images/atmel-42181-sam-d21_datasheet.pdf

  int cycles;
  uint8_t bits;

  if (!_watchdog_samd_initialized)
    _watchdog_samd_initialize_wdt();

#ifdef SAM_D5X_E5X
  WDT->CTRLA.reg = 0; // Disable watchdog for config
  while (WDT->SYNCBUSY.reg)
    ;
#else
  WDT->CTRL.reg = 0; // Disable watchdog for config
  while (WDT->STATUS.bit.SYNCBUSY)
    ;
#endif

  // You'll see some occasional conversion here compensating between
  // milliseconds (1000 Hz) and WDT clock cycles (~1024 Hz).  The low-
  // power oscillator used by the WDT ostensibly runs at 32,768 Hz with
  // a 1:32 prescale, thus 1024 Hz, though probably not super precise.

  if ((maxPeriodMS >= 16000) || !maxPeriodMS) {
    cycles = 16384;
    bits = 0xB;
  } else {
    cycles = (maxPeriodMS * 1024L + 500) / 1000; // ms -> WDT cycles
    if (cycles >= 8192) {
      cycles = 8192;
      bits = 0xA;
    } else if (cycles >= 4096) {
      cycles = 4096;
      bits = 0x9;
    } else if (cycles >= 2048) {
      cycles = 2048;
      bits = 0x8;
    } else if (cycles >= 1024) {
      cycles = 1024;
      bits = 0x7;
    } else if (cycles >= 512) {
      cycles = 512;
      bits = 0x6;
    } else if (cycles >= 256) {
      cycles = 256;
      bits = 0x5;
    } else if (cycles >= 128) {
      cycles = 128;
      bits = 0x4;
    } else if (cycles >= 64) {
      cycles = 64;
      bits = 0x3;
    } else if (cycles >= 32) {
      cycles = 32;
      bits = 0x2;
    } else if (cycles >= 16) {
      cycles = 16;
      bits = 0x1;
    } else {
      cycles = 8;
      bits = 0x0;
    }
  }

  // Watchdog timer on SAMD is a slightly different animal than on AVR.
  // On AVR, the WTD timeout is configured in one register and then an
  // interrupt can optionally be enabled to handle the timeout in code
  // (as in waking from sleep) vs resetting the chip.  Easy.
  // On SAMD, when the WDT fires, that's it, the chip's getting reset.
  // Instead, it has an "early warning interrupt" with a different set
  // interval prior to the reset.  For equivalent behavior to the AVR
  // library, this requires a slightly different configuration depending
  // whether we're coming from the sleep() function (which needs the
  // interrupt), or just enable() (no interrupt, we want the chip reset
  // unless the WDT is cleared first).  In the sleep case, 'windowed'
  // mode is used in order to allow access to the longest available
  // sleep interval (about 16 sec); the WDT 'period' (when a reset
  // occurs) follows this and is always just set to the max, since the
  // interrupt will trigger first.  In the enable case, windowed mode
  // is not used, the WDT period is set and that's that.
  // The 'earlyWarning' argument determines which behavior is used;
  // this isn't present in the AVR code, just here.  It defaults to
  // 'false' so existing Arduino code works as normal, while the sleep()
  // function (later in this file) explicitly passes 'true' to get the
  // alternate behavior.

#ifdef SAM_D5X_E5X
  if (earlyWarning) {
    WDT->INTFLAG.reg |= WDT_INTFLAG_EW; // Clear interrupt flag
    // WDT->INTFLAG.bit.EW = 1;        // Clear interrupt flag
    WDT->INTENSET.bit.EW = 1;       // Enable early warning interrupt
    WDT->CONFIG.bit.PER = 0xB;      // Period = max
    WDT->CONFIG.bit.WINDOW = bits;  // Set time of interrupt
    WDT->EWCTRL.bit.EWOFFSET = 0x0; // Early warning offset
    WDT->CTRLA.bit.WEN = 1;         // Enable window mode
    while (WDT->SYNCBUSY.reg)
      ; // Sync CTRL write
  } else {
    WDT->INTENCLR.bit.EW = 1;   // Disable early warning interrupt
    WDT->CONFIG.bit.PER = bits; // Set period for chip reset
    WDT->CTRLA.bit.WEN = 0;     // Disable window mode
    while (WDT->SYNCBUSY.reg)
      ; // Sync CTRL write
  }

  _watchdog_samd_reset();    // Clear watchdog interval
  WDT->CTRLA.bit.ENABLE = 1; // Start watchdog now!
  while (WDT->SYNCBUSY.reg)
    ;
#else
  if (earlyWarning) {
    WDT->INTENSET.bit.EW = 1;      // Enable early warning interrupt
    WDT->CONFIG.bit.PER = 0xB;     // Period = max
    WDT->CONFIG.bit.WINDOW = bits; // Set time of interrupt
    WDT->CTRL.bit.WEN = 1;         // Enable window mode
    while (WDT->STATUS.bit.SYNCBUSY)
      ; // Sync CTRL write
  } else {
    WDT->INTENCLR.bit.EW = 1;   // Disable early warning interrupt
    WDT->CONFIG.bit.PER = bits; // Set period for chip reset
    WDT->CTRL.bit.WEN = 0;      // Disable window mode
    while (WDT->STATUS.bit.SYNCBUSY)
      ; // Sync CTRL write
  }
#endif

  return (cycles * 1000L + 512) / 1024; // WDT cycles -> ms
}

void WDT_Handler(void) {
  // ISR for watchdog early warning, DO NOT RENAME!
#ifdef SAM_D5X_E5X
  WDT->CTRLA.bit.ENABLE = 0; // Disable watchdog
  while (WDT->SYNCBUSY.reg)
    ;
#else
  WDT->CTRL.bit.ENABLE = 0; // Disable watchdog
  while (WDT->STATUS.bit.SYNCBUSY)
    ; // Sync CTRL write
#endif
  WDT->INTFLAG.reg |= WDT_INTFLAG_EW; // Clear interrupt flag
  // WDT->INTFLAG.bit.EW = 1;        // Clear interrupt flag

  common_hal_mcu_watchdogtimer_obj.mode = WATCHDOGMODE_NONE;
  mp_obj_exception_clear_traceback(MP_OBJ_FROM_PTR(&mp_watchdog_timeout_exception));
  MP_STATE_VM(mp_pending_exception) = &mp_watchdog_timeout_exception;
#if MICROPY_ENABLE_SCHEDULER
  if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE) {
    MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
  }
#endif
}

void common_hal_watchdog_feed(watchdog_watchdogtimer_obj_t *self) {
    _watchdog_samd_reset();
}

void common_hal_watchdog_set_mode(watchdog_watchdogtimer_obj_t *self, watchdog_watchdogmode_t new_mode) {
    if (self->mode == new_mode) {
        return;
    }
    self->mode = new_mode;

    if (self->mode == WATCHDOGMODE_NONE) {
        _watchdog_samd_disable();
    } else {
        _watchdog_samd_enable(self->timeout * 1000, self->mode == WATCHDOGMODE_RAISE);
    }
}
watchdog_watchdogmode_t common_hal_watchdog_get_mode(watchdog_watchdogtimer_obj_t *self) {
    return self->mode;
}

void common_hal_watchdog_set_timeout(watchdog_watchdogtimer_obj_t *self, mp_float_t timeout) {
    self->timeout = timeout;

    if (self->mode == WATCHDOGMODE_NONE) {
        _watchdog_samd_disable();
    } else {
        _watchdog_samd_enable(self->timeout * 1000, self->mode == WATCHDOGMODE_RAISE);
    }
}
mp_float_t common_hal_watchdog_get_timeout(watchdog_watchdogtimer_obj_t *self) {
    return self->timeout;
}

void common_hal_watchdog_enable(watchdog_watchdogtimer_obj_t *self) {
    // unused
}
void common_hal_watchdog_deinit(watchdog_watchdogtimer_obj_t *self) {
    _watchdog_samd_disable();
}

void watchdog_reset(void) {
   common_hal_watchdog_deinit(&common_hal_mcu_watchdogtimer_obj);
}
