#include "pic.h"
#include "pic_public.h"
#include "ps2_mouse.h"
#include "ps2_mouse_handler.h"
#include "ps2_mouse_handler_internal.h"
#include "stdio.h"

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

	kprintf("port number = %u, mouse_irq = %u. Scan code = |%u:3, %h:4, %b:8|\n", port_number, mouse_irq, scancode, scancode, scancode);
	PIC_sendEOI(mouse_irq);
}
