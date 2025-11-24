#include "assert.h"
#include "pic.h"
#include "ps2_keyboard_handler.h"
#include "ps2_keyboard_public.h"
#include "stdio.h"
#include "vga_terminal.h"
// possible, create a ps2 controller device, and then a keyboard controller device. What was done here is essentially a keyboard hadnler device

extern struct keyboard_porting_state_context kps;

void parse_scan_code(uint8_t scancode);
void parse_extended_scan_code(uint8_t scancode);

void keyboard_handler_scs1(uint8_t scancode, uint8_t port_number);
void keyboard_handler_scs2(uint8_t scancode, uint8_t port_number);
void keyboard_handler_scs3(uint8_t scancode, uint8_t port_number);

extern inline void keyboard_handler(uint8_t scancode, uint8_t port_number) {
	switch (kps.active_scancode_set) {
	case 1:
		return keyboard_handler_scs1(scancode, port_number);
	case 2:
		return keyboard_handler_scs3(scancode, port_number);
	case 3:
		return keyboard_handler_scs3(scancode, port_number);
	default:
		abort_msg("Impossible scan scode set (%u)\n", kps.active_scancode_set);
	}
}

/*
Scancode: 		The mouse scancode sent, read from port 0x60. (PS2_DATA_PORT_RW)
port_number: 	The ps2 port number the mouse is connected to.
    This allows runtimes port number, and the keyboard isn't forced to be plugged in port1, with mouse in port2

Note: This function assume standard PS/2 mouse.



preconditions:
    Port Number must be 1 or 2

    Keyboard must be in scancode set 1.

*/
void keyboard_handler_scs1(uint8_t scancode, uint8_t port_number) {

	uint8_t keyboard_irq;
	if (port_number == 1) {
		keyboard_irq = PS2_PORT1_IRQ;
	} else {
		keyboard_irq = PS2_PORT2_IRQ;
	}
	// this can be optimized into a cmovs with immediates (no memory reads or branch)
	// Blazingly fast

	bool extended_signal = false;
	static bool previous_extended_signal = false;
	// TODO: Fix this.
	// This can break if we have two keyboards
	// A race condition can happened, since this is a shared ressource!

	bool press_not_release = false;
	if (scancode == 0xe0) {
		extended_signal = true;
	} else if (scancode < 128) {
		press_not_release = true;
	}

	// ===============
	if (extended_signal) {
		kprintf("scan code (e0       ) = |%u:3, %h:4, %b:8|\n", scancode, scancode, scancode);
		previous_extended_signal = extended_signal;
		PIC_sendEOI(keyboard_irq);
		return;
	}

	// ===============
	if (press_not_release) {
		kprintf("scan code (press    ) = |%u:3, %h:4, %b:8|\n", scancode, scancode, scancode);
	} else {

		// not press, not needed     |
		kprintf("scan code (release  ) = |%u:3, %h:4, %b:8|\n", scancode, scancode, scancode);
	}

	if (previous_extended_signal) {
		kprintf("This is a special key, with an e0 prefix\n");
	}

	if (!press_not_release) {
		terminal_putchar('\n');
	}
	// ===============

	if (previous_extended_signal) {
		parse_extended_scan_code(scancode);
	} else {
		// Execute the code for non special keys.
		parse_scan_code(scancode);
	}

	previous_extended_signal = extended_signal;
	PIC_sendEOI(keyboard_irq);
}

void keyboard_handler_scs2(uint8_t scancode, uint8_t port_number) {
	kprintf("[PANIC-WARNING] scan code 2 handling not supported!\n");
}

void keyboard_handler_scs3(uint8_t scancode, uint8_t port_number) {
	kprintf("[PANIC-WARNING] scan code 3 handling not supported!\n");
}

void parse_scan_code(uint8_t scancode) {
}
void parse_extended_scan_code(uint8_t scancode) {
}
