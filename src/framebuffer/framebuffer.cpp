#include "assert.h"
#include "framebuffer.h"
#include "framebuffer.hpp"
#include "stdio.h"
#include <stdint.h>

struct framebuffer_t
{
    volatile struct color_t *base_address;
    uint32_t                 width;
    uint32_t                 height;
    uint32_t                 pitch;
};

framebuffer_t g_framebuffer;

// Create from individual RGBA components
constexpr struct color_t create_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return color_t{b, g, r, a};
}

// Create from a single 32-bit hex value: 0xAARRGGBB
constexpr struct color_t create_color(uint32_t color_hex)
{
    return color_t{
        static_cast<uint8_t>((color_hex >> 8) & 0xFF),  // B
        static_cast<uint8_t>((color_hex >> 16) & 0xFF), // G
        static_cast<uint8_t>((color_hex >> 24) & 0xFF), // R
        static_cast<uint8_t>(color_hex & 0xFF)          // A
    };
}

extern inline int framebuffer::set_pixel(uint32_t x, uint32_t y, color_t color)
{

    uint32_t width  = g_framebuffer.width;
    uint32_t height = g_framebuffer.height;
    uint32_t pitch  = g_framebuffer.pitch;

#ifdef DEBUG
    if (x > width)
    {
        return 1;
    }
    if (y > height)
    {
        return 2;
    }
#endif

    uint32_t idx                      = y * width + x;
    g_framebuffer.base_address[idx].r = color.r;
    g_framebuffer.base_address[idx].g = color.g;
    g_framebuffer.base_address[idx].b = color.b;
    g_framebuffer.base_address[idx].a = color.a;
    // should compile to mov [base + idx*4], %reg

    return 0;
}

int framebuffer::draw_horizontal_line(uint32_t x_start, uint32_t x_end, uint32_t y, color_t color)
{
    for (uint32_t x = x_start; x < x_end; x++)
    {
        uint32_t ret = framebuffer::set_pixel(x, y, color);

#ifdef DEBUG
        if (ret)
        {
            return ret;
        }
#endif
    }
    return 0;
}

int framebuffer::draw_vertical_line(uint32_t x, uint32_t y_start, uint32_t y_end, color_t color)
{
    for (uint32_t y = y_start; y < y_end; y++)
    {
        uint32_t ret = framebuffer::set_pixel(x, y, color);

#ifdef DEBUG
        if (ret)
        {
            return ret;
        }
#endif
    }
    return 0;
}

int framebuffer::draw_rectangle(uint32_t top_left_x, uint32_t top_left_y, uint32_t width, uint32_t height, color_t color)
{

    for (uint32_t x = top_left_x; x < top_left_x + width; x++)
    {
        for (uint32_t y = top_left_y; y < top_left_y + height; y++)
        {
            uint32_t ret = framebuffer::set_pixel(x, y, color);

#ifdef DEBUG
            if (ret)
            {
                return ret;
            }
#endif
        }
    }
    return 0;
}

void draw_3_lines(uint8_t row_c)
{
    for (int i = 0; i < row_c; i++)
    {
        framebuffer::draw_horizontal_line(0, g_framebuffer.width, i, color_t{0x00, 0x00, 0xff, 0xff});
        framebuffer::draw_horizontal_line(0, g_framebuffer.width, row_c + i, color_t{0x00, 0xff, 0x00, 0xff});
        framebuffer::draw_horizontal_line(0, g_framebuffer.width, 2 * row_c + i, color_t{0xff, 0x00, 0x00, 0xff});
    }
}

void framebuffer::do_test(uint32_t width, uint32_t height, uint32_t pitch, volatile struct color_t *base_address)

{
    assert(base_address != nullptr, "Null Base Address\n");
    g_framebuffer.base_address = base_address;
    g_framebuffer.height       = height;
    g_framebuffer.width        = width;
    g_framebuffer.pitch        = pitch;
    draw_3_lines(5);

    draw_rectangle(50, 200, 30, 180, color_t{0xff, 0x0, 0xff, 0x0});
}

#ifdef __cplusplus
extern "C"
{
#endif

    void do_test_c(volatile struct color_t *base_address, uint32_t width, uint32_t height, uint32_t pitch)
    {

        framebuffer::do_test(width, height, pitch, base_address);
    }

#ifdef __cplusplus
}
#endif
