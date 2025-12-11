#pragma once
#include "framebuffer.h"
#include <stdint.h>

namespace framebuffer
{

// Single pixel draw — simple enough, keep as-is
int set_pixel(uint16_t x, uint16_t y, color_t color);

// Horizontal line
struct draw_horizontal_line_args
{
    uint16_t y;
    uint16_t x_start;
    uint16_t x_end;
    color_t  color;
    uint16_t thickness = 1;
};
int draw_horizontal_line(struct draw_horizontal_line_args args);

// Vertical line
struct draw_vertical_line_args
{
    uint16_t x;
    uint16_t y_start;
    uint16_t y_end;
    color_t  color;
    uint16_t thickness = 1;
};
int draw_vertical_line(struct draw_vertical_line_args args);

// Rectangle
struct draw_rectangle_args
{
    uint16_t top_left_x;
    uint16_t top_left_y;
    uint16_t width;
    uint16_t height;
    color_t  color;
};
// uint32_t top_left_x;  uint32_t top_left_y; uint32_t width; uint32_t height; color_t  color;
int draw_rectangle(struct draw_rectangle_args args);

struct bitmap
{
    uint16_t height;
    uint16_t width;
    uint16_t effective_pitch; // = pitch / sizeof(color_t)
    color_t *data;
};

struct draw_bitmap_args
{
    uint16_t      top_left_x;
    uint16_t      top_left_y;
    struct bitmap bitmap;
};
// uint32_t top_left_x;  uint32_t top_left_y; uint32_t width; uint32_t height; color_t  color;
int draw_bitmap(struct draw_bitmap_args args);

// Test function
void do_test(volatile struct color_t *base_address, uint16_t width, uint16_t height, uint16_t pitch);

} // namespace framebuffer
