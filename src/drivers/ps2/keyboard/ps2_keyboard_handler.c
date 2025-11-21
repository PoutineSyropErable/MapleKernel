#include "pic.h"
#include "ps2_keyboard_handler.h"
#include "stdio.h"
#include "vga_terminal.h"
// possible, create a ps2 controller device, and then a keyboard controller device. What was done here is essentially a keyboard hadnler device

void parse_scan_code(uint8_t scancode) {
}

void parse_extended_scan_code(uint8_t scancode) {
}

void keyboard_handler(uint8_t scancode, uint8_t port_number) {

	uint8_t keyboard_irq = irq_of_ps2_port[port_number];

	bool extended_signal = false;
	static bool previous_extended_signal = false;
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
