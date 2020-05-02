#include "shared-bindings/displayio/Rectangle.h"

#include <string.h>

#include "py/runtime.h"

void common_hal_displayio_rectangle_construct(displayio_rectangle_t *self, uint32_t width, uint32_t height) {
    self->width = width;
    self->height = height;
}

uint32_t common_hal_displayio_rectangle_get_pixel(void *obj, int16_t x, int16_t y) {
    displayio_rectangle_t *self = obj;
    if (x < 0 || x >= self->width || y >= self->height || y < 0) {
        return 0;
    }
    return 1;
}
