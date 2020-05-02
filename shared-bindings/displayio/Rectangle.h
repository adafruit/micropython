#ifndef MICROPY_INCLUDED_SHARED_BINDINGS_DISPLAYIO_RECTANGLE_H
#define MICROPY_INCLUDED_SHARED_BINDINGS_DISPLAYIO_RECTANGLE_H

#include "shared-module/displayio/Rectangle.h"

extern const mp_obj_type_t displayio_rectangle_type;

void common_hal_displayio_rectangle_construct(displayio_rectangle_t *self, uint32_t width, uint32_t height);

uint32_t common_hal_displayio_rectangle_get_pixel(void *shape, int16_t x, int16_t y);

#endif // MICROPY_INCLUDED_SHARED_BINDINGS_DISPLAYIO_RECTANGLE_H
