// kernel_cpp.cpp
#include "kernel_cpp.h"
#include "kernel_cpp.hpp"
#include "stdio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// #include "framebuffer.h"
#include "framebuffer.hpp"
#include "ps2_mouse_handler.h"

int cpp_event_loop(void)
{
	static int16_t			  old_x = 0;
	static int16_t			  old_y = 0;
	static framebuffer::Color old_color(0xff0000);
	static framebuffer::Color current_color(0x00ff00);
	static bool				  first_itt = true;

	// The order is important
	if (!first_itt)
		framebuffer::g_framebuffer.set_pixel(old_x, old_y, old_color);
	current_color = framebuffer::g_framebuffer.get_pixel(g_mouse_prop.x, g_mouse_prop.y);
	// current_color.print();
	framebuffer::g_framebuffer.set_pixel(g_mouse_prop.x, g_mouse_prop.y, framebuffer::Color(0xffffff));

	old_x	  = g_mouse_prop.x;
	old_y	  = g_mouse_prop.y;
	old_color = current_color;
	first_itt = false;
	return 0;
}
