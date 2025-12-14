#include "assert.h"
#include "framebuffer.h"
#include "framebuffer.hpp"
#include "math.h"
#include "math.hpp"
#include "stdio.h"
#include <stdint.h>

using namespace framebuffer;

FrameBuffer framebuffer::g_framebuffer;

FrameBuffer::FrameBuffer() noexcept
{
}

void FrameBuffer::initialize(volatile Color *base, uint16_t w, uint16_t h, uint16_t eff_pitch)
{
	base_address	= base;
	width			= w;
	height			= h;
	effective_pitch = eff_pitch;
}

int FrameBuffer::draw_horizontal_line(const struct DrawHorizontalLineArgs &args)
{

	for (uint16_t thick_offset = 0; thick_offset < args.thickness; thick_offset++)
	{

		for (uint32_t x = args.x_start; x < args.x_end; x++)
		{
			[[maybe_unused]] uint32_t ret = set_pixel(x, args.y + thick_offset, args.color);

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

int FrameBuffer::draw_vertical_line(const DrawVerticalLineArgs &args)
{
	kprintf("red: %h, green: %h, blue: %h, a:%h\n", args.color.r, args.color.g, args.color.b, args.color.a);

	for (uint16_t y = args.y_start; y < args.y_end; y++)
	{
		for (uint16_t thick_offset = 0; thick_offset < args.thickness; thick_offset++)
		{
			[[maybe_unused]] uint32_t ret = set_pixel(args.x + thick_offset, y, args.color);
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

void FrameBuffer::draw_line(const DrawLineArgs &args)
{
	float dx = args.x1 - args.x0;
	float dy = args.y1 - args.y0;

	float dx_abs = abs(dx);
	float dy_abs = abs(dy);

	if (dy == 0)
	{
		draw_horizontal_line({.y = args.y0, .x_start = args.x0, .x_end = args.x1, .color = args.color});
		return;
	}
	else if (dx == 0)
	{
		draw_vertical_line({.x = args.x0, .y_start = args.y0, .y_end = args.y1, .color = args.color});
		return;
	}

	if (dy_abs <= dx_abs)
	{
		// nearly horizontal line, regular cartesian;
		float m = dy / dx;
		float b = args.y0 - args.x0 * m;
	}
	else
	{
		// Nearly vertical line. Divide by dy
		float m_i = dx / dy;
		float b_i = args.x0 - args.y0 * m_i;
	}

	// Temporary method?
	float l = sqrt(dx * dx + dy * dy);
	for (double t = 0; t < 1; t += 1 / l)
	{
		double x = args.x0 + dx * t;
		double y = args.y0 + dy * t;
		set_pixel(x, y, args.color);
	}
}

void FrameBuffer::draw_line_quick(const DrawLineArgs &args)
{
	int x0 = args.x0, y0 = args.y0;
	int x1 = args.x1, y1 = args.y1;

	int dx	= abs(x1 - x0);
	int dy	= abs(y1 - y0);
	int sx	= (x0 < x1) ? 1 : -1;
	int sy	= (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while (true)
	{
		set_pixel(x0, y0, args.color);
		if (x0 == x1 && y0 == y1)
			break;
		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

int FrameBuffer::draw_rectangle(const DrawRectangleArgs &args)
{

	for (uint32_t y = args.top_left_y; y < args.top_left_y + args.height; y++)
	{
		for (uint32_t x = args.top_left_x; x < args.top_left_x + args.width; x++)
		{
			[[maybe_unused]] uint32_t ret = set_pixel(x, y, args.color);

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

int FrameBuffer::draw_bitmap(const DrawBitmapArgs &args)
{
	const struct Bitmap &bitmap = args.bitmap;
	for (uint32_t j = 0; j < args.bitmap.height; j++)
	{
		for (uint32_t i = 0; i < args.bitmap.width; i++)
		{
			uint16_t x = args.top_left_x + i;
			uint16_t y = args.top_left_y + j;

			// bitmap pitch = width *4
			uint32_t				  b_idx = j * bitmap.effective_pitch + i;
			[[maybe_unused]] uint32_t ret	= set_pixel(x, y, args.bitmap.data[b_idx]);

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

// End of class function declaration

void draw_rgb_lines(uint8_t line_thickness, uint16_t starting_y)
{
	// Red line
	g_framebuffer.draw_horizontal_line({.y = (uint16_t)(0 * line_thickness + starting_y),
		.x_start						   = 0,
		.x_end							   = g_framebuffer.get_width(),
		.color							   = Color(0xff0000),
		.thickness						   = line_thickness});

	// Green line
	g_framebuffer.draw_horizontal_line({.y = (uint16_t)(1 * line_thickness + starting_y),
		.x_start						   = 0,
		.x_end							   = g_framebuffer.get_width(),
		.color							   = Color(0x00ff00),
		.thickness						   = line_thickness});

	// Blue line
	g_framebuffer.draw_horizontal_line({.y = (uint16_t)(2 * line_thickness + starting_y),
		.x_start						   = 0,
		.x_end							   = g_framebuffer.get_width(),
		.color							   = Color(0x0000ff),
		.thickness						   = line_thickness});
}

void do_test(volatile struct color_t *base_address, uint16_t width, uint16_t height, uint16_t pitch)
{

	assert(base_address != nullptr, "Null Base Address\n");
	volatile framebuffer::Color *cpp_base = reinterpret_cast<volatile framebuffer::Color *>(base_address);
	g_framebuffer.initialize(cpp_base, width, height, pitch / sizeof(Color));

	draw_rgb_lines(10, g_framebuffer.get_height() / 2 - 5);
	g_framebuffer.draw_rectangle({.top_left_x = 50, .top_left_y = 190, .width = 30, .height = 180, .color = Color(0xff00ff)});

	g_framebuffer.draw_vertical_line(
		{.x = 800, .y_start = 0, .y_end = g_framebuffer.get_height(), .color = Color(0xffff00), .thickness = 2});

	g_framebuffer.draw_horizontal_line(
		{.y = 500, .x_start = 0, .x_end = g_framebuffer.get_width(), .color = Color(0x329cf4), .thickness = 3});
}

#ifdef __cplusplus
extern "C"
{
#endif

	void do_test_c(volatile struct color_t *base_address, uint32_t width, uint32_t height, uint32_t pitch)
	{

		do_test(base_address, width, height, pitch);
	}

#ifdef __cplusplus
}
#endif
