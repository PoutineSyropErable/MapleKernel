#pragma once
#include "framebuffer.h"
#include <stdint.h>

namespace framebuffer
{

int set_pixel(uint32_t x, uint32_t y, color_t color);

int draw_horizontal_line(uint32_t x_start, uint32_t x_end, uint32_t y, color_t color);
int draw_vertical_line(uint32_t x, uint32_t y_start, uint32_t y_end, color_t color);

int draw_rectangle(uint32_t top_left_x, uint32_t top_left_y, uint32_t width, uint32_t height, color_t color);

void do_test(uint32_t width, uint32_t height, uint32_t pitch, volatile struct color_t *base_address);

} // namespace framebuffer
