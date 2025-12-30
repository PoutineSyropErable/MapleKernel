#include "apic/apic.hpp"
#include "assert.h"
#include "pic.h"
#include "pic_public.h"
#include "ps2_mouse.h"
#include "ps2_mouse_handler.h"
#include "ps2_mouse_handler_internal.hpp"
#include "stdio.h"

#include "framebuffer.hpp"

#define packet_3_size sizeof(ps2_mouse::generic_3_packet)

volatile struct mouse_properties g_mouse_prop;
namespace
{
ps2_mouse::generic_3_packet global_3_packet{};
uint8_t						packet_idx = 0;
struct mouse_button_pressed
{
	bool left_button : 1;
	bool middle_button : 1;
	bool right_button : 1;
	bool mouse_4 : 1;
	bool mouse_5 : 1;
} mouse_button_pressed;
} // namespace

static inline int16_t sign_extend(uint8_t val, bool sign)
{
	if (sign)
		return val | 0xFF00;
	else
		return val;
}

/*
Scancode: 		The mouse scancode sent, read from port 0x60. (PS2_DATA_PORT_RW)
port_number: 	The ps2 port number the mouse is connected to.
	This allows runtimes port number, and the keyboard isn't forced to be plugged in port1, with mouse in port2

Note: This function assume standard PS/2 mouse.



preconditions:
	Port Number must be 1 or 2!

*/
void mouse_handler(uint8_t scancode, uint8_t port_number)
{

	uint8_t mouse_irq;
	if (port_number == 1)
	{
		mouse_irq = PS2_PORT1_IRQ;
	}
	else
	{
		mouse_irq = PS2_PORT2_IRQ;
	}
	// This code can be optimised to a cmov, and is faster then array access.
	// Since it needs no memory read. Since they are constants

	// kprintf("port number = %u, mouse_irq = %u. Scan code = |%u:3, %h:4, %b:8|\n", port_number, mouse_irq, scancode, scancode, scancode);

	switch (packet_idx)
	// This switch satement should compile to an array move. Since case n does move
	{
	case 0: global_3_packet.flags = ps2_mouse::first_packet(scancode); break;

	case 1: global_3_packet.x_axis_movement = scancode; break;

	case 2: global_3_packet.y_axis_movement = scancode; break;

#ifdef DEBUG
	default: abort_msg("Impossible case\n");
#endif
	}

	if (packet_idx == (packet_3_size - 1))
	{

		uint16_t x_s = global_3_packet.flags.x_axis_sign;
		uint16_t y_s = global_3_packet.flags.y_axis_sign;
		uint8_t	 dx8 = global_3_packet.x_axis_movement;
		uint8_t	 dy8 = global_3_packet.y_axis_movement;

		int16_t dx = sign_extend(dx8, x_s);
		int16_t dy = sign_extend(dy8, y_s);

		if (global_3_packet.flags.x_axis_overflow)
		{
			dx = 0;
		}
		if (global_3_packet.flags.y_axis_overflow)
		{
			dy = 0;
		}

		struct mouse_button_event
		{
			bool left_button_first_pressed	 = 0;
			bool right_button_first_pressed	 = 0;
			bool middle_button_first_pressed = 0;

			bool left_button_pressed   = 0;
			bool right_button_pressed  = 0;
			bool middle_button_pressed = 0;

			bool left_button_released	= 0;
			bool right_button_released	= 0;
			bool middle_button_released = 0;

		} mouse_button_event;

		// ---- Note: No interrupt are sent on continued press.
		// ================== Left button =====================
		if (global_3_packet.flags.button_left)
		{
			if (!mouse_button_pressed.left_button)
			{
				mouse_button_event.left_button_first_pressed = true;
				kprintf("[First Press] Left mouse button\n");
			}
			mouse_button_pressed.left_button	   = true;
			mouse_button_event.left_button_pressed = true;
		}
		else
		{
			if (mouse_button_pressed.left_button)
			{
				mouse_button_event.left_button_released = true;
				kprintf("[Released  ] Left mouse button\n");
			}
			mouse_button_pressed.left_button	   = false;
			mouse_button_event.left_button_pressed = false;
		}

		// ================== Middle button =====================
		if (global_3_packet.flags.button_middle)
		{
			if (!mouse_button_pressed.middle_button)
			{
				mouse_button_event.middle_button_first_pressed = true;
				kprintf("[First Press] Middle mouse button\n");
			}
			mouse_button_pressed.middle_button		 = true;
			mouse_button_event.middle_button_pressed = true;
		}
		else
		{
			if (mouse_button_pressed.middle_button)
			{
				mouse_button_event.middle_button_released = true;
				kprintf("[Released  ] Middle mouse button\n");
			}
			mouse_button_pressed.middle_button		 = false;
			mouse_button_event.middle_button_pressed = false;
		}

		// ================== Right button =====================
		if (global_3_packet.flags.button_right)
		{
			if (!mouse_button_pressed.right_button)
			{
				mouse_button_event.right_button_first_pressed = true;
				kprintf("[First Press] Right mouse button\n");
			}
			mouse_button_pressed.right_button		= true;
			mouse_button_event.right_button_pressed = true;
		}
		else
		{
			if (mouse_button_pressed.right_button)
			{
				mouse_button_event.right_button_released = true;
				kprintf("[Released  ] Right mouse button\n");
			}
			mouse_button_pressed.right_button		= false;
			mouse_button_event.right_button_pressed = false;
		}

		packet_idx = 0;

		bool move_only = !mouse_button_event.left_button_first_pressed && !mouse_button_event.left_button_released &&
						 !mouse_button_event.middle_button_first_pressed && !mouse_button_event.middle_button_released &&
						 !mouse_button_event.right_button_first_pressed && !mouse_button_event.right_button_released;

		if (move_only)
		{
			kprintf("xmovement: %d, y_movement: %d\n\n", dx, dy);
		}

		uint16_t old_x = g_mouse_prop.x;
		uint16_t old_y = g_mouse_prop.y;

		g_mouse_prop.x += dx;
		g_mouse_prop.y -= dy; // (since this means y = up. But for screen, y = down)
		if (g_mouse_prop.x < 0)
		{
			g_mouse_prop.x = 0;
		}

		if (g_mouse_prop.y < 0)
		{
			g_mouse_prop.y = 0;
		}

		if (mouse_button_event.left_button_pressed)
		{
			framebuffer::g_framebuffer.draw_line_quick({.x0 = old_x,
				.y0											= old_y,
				.x1											= static_cast<uint16_t>(g_mouse_prop.x),
				.y1											= static_cast<uint16_t>(g_mouse_prop.y),
				.color										= {g_mouse_prop.b, g_mouse_prop.g, g_mouse_prop.r, g_mouse_prop.a}});
		}
		if (mouse_button_event.right_button_pressed)
		{
			framebuffer::Color color_of_mouse = framebuffer::g_framebuffer.get_pixel(g_mouse_prop.x + 1, g_mouse_prop.y);
			// + 1 is a temporary hack because the trail pixel is there. Normally, use two buffers.
			g_mouse_prop.r = color_of_mouse.r;
			g_mouse_prop.g = color_of_mouse.g;
			g_mouse_prop.b = color_of_mouse.b;
			g_mouse_prop.a = color_of_mouse.a;
		}
	}
	else
	{
		packet_idx++;
	}

	// PIC_sendEOI(mouse_irq);
	apic::send_eoi();
}
