#ifndef MICROPY_INCLUDED_SHARED_MODULE_DISPLAYIO_RECTANGLE_H
#define MICROPY_INCLUDED_SHARED_MODULE_DISPLAYIO_RECTANGLE_H

#include <stdint.h>
#include <float.h>

#include "py/obj.h"

typedef struct {
    mp_obj_base_t base;
    uint16_t width;
    uint16_t height;
} displayio_rectangle_t;

#endif // MICROPY_INCLUDED_SHARED_MODULE_DISPLAYIO_RECTANGLE_H
