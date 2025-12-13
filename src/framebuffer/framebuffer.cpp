#include "assert.h"
#include "framebuffer.h"
#include "framebuffer.hpp"
#include "stdio.h"
#include <stdint.h>

struct framebuffer_t
{
	volatile struct color_t *base_address;
	uint16_t				 width;
	uint16_t				 height;
	uint16_t				 effective_pitch; // use this for c[x][y] = c[effective_pitch*y + x]
};

framebuffer_t  g_framebuffer;
framebuffer_t &g_fb = g_framebuffer;

// Create from individual RGBA components
constexpr struct color_t create_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return color_t{b, g, r, a};
}

// Create from a single 32-bit hex value: 0xAARRGGBB.
// On little endian system, this is the same ordering.
// On big endian, the one bellow is needed
constexpr struct color_t create_color(uint32_t color_hex)
{
	union color_uts
	{
		color_t	 color;
		uint32_t raw;
	} ret;

	ret.raw = color_hex;
	return ret.color;

	// return color_t{
	//     static_cast<uint8_t>((color_hex >> 0) & 0xFF),  // B
	//     static_cast<uint8_t>((color_hex >> 8) & 0xFF),  // G
	//     static_cast<uint8_t>((color_hex >> 16) & 0xFF), // R
	//     static_cast<uint8_t>((color_hex >> 24) & 0xFF)  // A
	// };
}

extern inline int framebuffer::set_pixel(uint16_t x, uint16_t y, color_t color)
{

	[[maybe_unused]] uint32_t width			  = g_framebuffer.width;
	[[maybe_unused]] uint32_t height		  = g_framebuffer.height;
	uint32_t				  effective_pitch = g_framebuffer.effective_pitch;

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

	// Pitch is counted in byte count, not dword counts.
	// So, Pitch >= Width * 4

	uint32_t idx					  = y * effective_pitch + x;
	g_framebuffer.base_address[idx].r = color.r;
	g_framebuffer.base_address[idx].g = color.g;
	g_framebuffer.base_address[idx].b = color.b;
	g_framebuffer.base_address[idx].a = color.a;
	// should compile to mov [base + idx*4], %reg

	return 0;
}

int framebuffer::draw_horizontal_line(struct draw_horizontal_line_args args)
{

	for (uint16_t thick_offset = 0; thick_offset < args.thickness; thick_offset++)
	{

		for (uint32_t x = args.x_start; x < args.x_end; x++)
		{
			[[maybe_unused]] uint32_t ret = framebuffer::set_pixel(x, args.y + thick_offset, args.color);

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

int framebuffer::draw_vertical_line(struct draw_vertical_line_args args)
{

	kprintf("red: %h, green: %h, blue: %h, a:%h\n", args.color.r, args.color.g, args.color.b, args.color.a);

	for (uint16_t y = args.y_start; y < args.y_end; y++)
	{
		for (uint16_t thick_offset = 0; thick_offset < args.thickness; thick_offset++)
		{
			[[maybe_unused]] uint32_t ret = framebuffer::set_pixel(args.x + thick_offset, y, args.color);
		}

#ifdef DEBUG
		if (ret)
		{
			return ret;
		}
#endif
	}
	return 0;
}

int framebuffer::draw_rectangle(struct draw_rectangle_args args)
{

	for (uint32_t y = args.top_left_y; y < args.top_left_y + args.height; y++)
	{
		for (uint32_t x = args.top_left_x; x < args.top_left_x + args.width; x++)
		{
			[[maybe_unused]] uint32_t ret = framebuffer::set_pixel(x, y, args.color);

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

int framebuffer::draw_bitmap(struct draw_bitmap_args args)
{

	struct bitmap &bitmap = args.bitmap;
	for (uint32_t j = 0; j < args.bitmap.height; j++)
	{
		for (uint32_t i = 0; i < args.bitmap.width; i++)
		{
			uint16_t x = args.top_left_x + i;
			uint16_t y = args.top_left_y + j;

			// bitmap pitch = width *4
			uint32_t				  b_idx = j * bitmap.effective_pitch + i;
			[[maybe_unused]] uint32_t ret	= framebuffer::set_pixel(x, y, args.bitmap.data[b_idx]);

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

void draw_rgb_lines(uint8_t line_thickness, uint16_t starting_y)
{
	// Red line
	framebuffer::draw_horizontal_line({.y = (uint16_t)(0 * line_thickness + starting_y),
		.x_start						  = 0,
		.x_end							  = g_framebuffer.width,
		.color							  = create_color(0xff0000),
		.thickness						  = line_thickness});

	// Green line
	framebuffer::draw_horizontal_line({.y = (uint16_t)(1 * line_thickness + starting_y),
		.x_start						  = 0,
		.x_end							  = g_framebuffer.width,
		.color							  = create_color(0x00ff00),
		.thickness						  = line_thickness});

	// Blue line
	framebuffer::draw_horizontal_line({.y = (uint16_t)(2 * line_thickness + starting_y),
		.x_start						  = 0,
		.x_end							  = g_framebuffer.width,
		.color							  = create_color(0x0000ff),
		.thickness						  = line_thickness});
}

void framebuffer::do_test(volatile struct color_t *base_address, uint16_t width, uint16_t height, uint16_t pitch)

{
	assert(base_address != nullptr, "Null Base Address\n");
	g_framebuffer.base_address	  = base_address;
	g_framebuffer.height		  = height;
	g_framebuffer.width			  = width;
	g_framebuffer.effective_pitch = pitch / sizeof(color_t);

	draw_rgb_lines(10, g_fb.height / 2 - 5);
	draw_rectangle({.top_left_x = 50, .top_left_y = 190, .width = 30, .height = 180, .color = create_color(0xff00ff)});

	framebuffer::draw_vertical_line(
		{.x = 800, .y_start = 0, .y_end = g_framebuffer.height, .color = create_color(0xffff00), .thickness = 2});

	framebuffer::draw_horizontal_line(
		{.y = 500, .x_start = 0, .x_end = g_framebuffer.width, .color = create_color(0x329cf4), .thickness = 3});
}

#ifdef __cplusplus
extern "C"
{
#endif

	void do_test_c(volatile struct color_t *base_address, uint32_t width, uint32_t height, uint32_t pitch)
	{

		framebuffer::do_test(base_address, width, height, pitch);
	}

#ifdef __cplusplus
}
#endif
