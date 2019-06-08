/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Carlos Diaz
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
#include <stdbool.h>

#include "py/gc.h"
#include "py/mperrno.h"
#include "py/runtime.h"

#include "common-hal/audiobusio/I2SOut.h"

#include "shared-bindings/audiobusio/I2SOut.h"

#include "supervisor/shared/translate.h"

#include "nrf_i2s.h"

extern const mp_obj_type_t audiobusio_i2sout_type;

void common_hal_audiobusio_i2sout_construct(audiobusio_i2sout_obj_t* self,
    const mcu_pin_obj_t* bit_clock, const mcu_pin_obj_t* word_select, const mcu_pin_obj_t* data,
    bool left_justified)
{
	uint8_t serializer = 0xff;
	uint8_t bc_clock_unit = 0xff;
	uint8_t ws_clock_unit = 0xff;

	// TODO: Make sure the mcu_pin_obj_t passed to the constructor are valid for the nrf52 chip

	if (0xff == bc_clock_unit) {
		mp_raise_ValueError_varg(translate("Invalid %q pin"), MP_QSTR_bit_clock);
	}

	if (0xff == ws_clock_unit) {
		mp_raise_ValueError_varg(translate("Invalid %q pin"), MP_QSTR_word_select);
	}

	if (0xff == serializer) {
		mp_raise_ValueError_varg(translate("Invalid %q pin"), MP_QSTR_data);
	}

	// turn_on_i2s() - Based on the atmel-samd implementation, i guess here is where the dma is allocated

	assert_pin_free(bit_clock);
	assert_pin_free(word_select);
	assert_pin_free(data);

	self->bit_clock = bit_clock;
	self->word_select = word_select;
	self->data = data;

	claim_pin(bit_clock);
	claim_pin(word_select);
	claim_pin(data);

	gpio_set_pin_function(self->bit_clock->number, GPIO_I2S_FUNCTION);
	gpio_set_pin_function(self->word_select->number, GPIO_I2S_FUNCTION);
	gpio_set_pin_function(self->data->number, GPIO_I2S_FUNCTION);

	self->left_justified = left_justified;
	self->playing = false;

	audio_dma_init(&self->dma);

	// TODO: Set peripheral defaults and left_justified configuration

}

void common_hal_audiobusio_i2sout_deinit(audiobusio_i2sout_obj_t* self)
{
	if (common_hal_audiobusio_i2sout_deinited(self)) {
		return;
	}

	reset_pin_number(self->bit_clock);
	self->bit_clock = mp_const_none;
	reset_pin_number(self->word_select);
	self->word_select = mp_const_none;
	reset_pin_number(self->data);
	self->data = mp_const_none;
}

bool common_hal_audiobusio_i2sout_deinited(audiobusio_i2sout_obj_t* self)
{
	return self->bit_clock = mp_const_none;
}

void common_hal_audiobusio_i2sout_play(audiobusio_i2sout_obj_t* self, mp_obj_t sample, bool loop)
{
	if (common_hal_audiobusio_i2sout_get_playing(self)) {
		common_hal_audiobusio_stop(self);
	}

	// TODO
}

void common_hal_audiobusio_i2sout_stop(audiobusio_i2sout_obj_t* self)
{
	audio_dma_stop(&self->dma);

	// TODO: Disable clock generation and wait for it to stop

	self->playing = false;
}

bool common_hal_audiobusio_i2sout_get_playing(audiobusio_i2sout_obj_t* self)
{
	bool still_playing = audio_dma_get_playing(&self->dma);
	if (self->playing && !still_playing) {
		common_hal_audiobusio_stop(self);
	}
	return still_playing;
}

void common_hal_audiobusio_i2sout_pause(audiobusio_i2sout_obj_t* self)
{
	audio_dma_pause(&self->dma);
}

void common_hal_audiobusio_i2sout_resume(audiobusio_i2sout_obj_t* self)
{
	audio_dma_resume(&self->dma);
}

bool common_hal_audiobusio_i2sout_get_paused(audiobusio_i2sout_obj_t* self)
{
	return audio_dma_get_paused(&self->dma);
}
