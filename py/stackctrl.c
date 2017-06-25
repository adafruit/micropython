/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Paul Sokolovsky
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

#include "py/mpstate.h"
#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/stackctrl.h"

MP_NOINLINE void mp_stack_ctrl_init(void) {
    // Force routine to not be inlined. Better guarantee than MP_NOINLINE for -flto.
    asm("");
    volatile int stack_dummy;
    MP_STATE_THREAD(stack_top) = (char*)&stack_dummy;
}

void mp_stack_set_top(void *top) {
    MP_STATE_THREAD(stack_top) = top;
}

MP_NOINLINE mp_uint_t mp_stack_usage(void) {
    // Assumes descending stack
    // Force routine to not be inlined. Better guarantee than MP_NOINLINE for -flto.
    asm("");
    volatile int stack_dummy;
    return MP_STATE_THREAD(stack_top) - (char*)&stack_dummy;
}

#if MICROPY_STACK_CHECK

void mp_stack_set_limit(mp_uint_t limit) {
    MP_STATE_THREAD(stack_limit) = limit;
}

void mp_exc_recursion_depth(void) {
    mp_raise_RuntimeError("maximum recursion depth exceeded");
}

void mp_stack_check(void) {
    if (mp_stack_usage() >= MP_STATE_THREAD(stack_limit)) {
        mp_exc_recursion_depth();
    }
}

#endif // MICROPY_STACK_CHECK

#if MICROPY_PY_UMEM_MAX_STACK_USE

// End of bss section. Stack cannot go further than this.
const char MP_MAX_STACK_USE_SENTINEL_BYTE = 0xEE;

// Record absolute bottom (logical limit) of stack.
void mp_stack_set_bottom(void* stack_bottom) {
    MP_STATE_THREAD(stack_bottom) = stack_bottom;
}

// Fill stack space down toward the stack limit with a known unusual value.
MP_NOINLINE void mp_stack_fill_with_sentinel(void) {
    // Force routine to not be inlined. Better guarantee than MP_NOINLINE for -flto.
    asm("");
    volatile char* volatile p;
    // Start filling stack just below the last variable in the current stack frame, which is p.
    // Continue until we've hit the bottom of the stack (lowest address, logical "ceiling" of stack).
    p = (char *) (&p - 1);
    while(p >= MP_STATE_THREAD(stack_bottom)) {
	*p-- = MP_MAX_STACK_USE_SENTINEL_BYTE;
    }
}

#endif // MICROPY_PY_UMEM_MAX_STACK_USE
