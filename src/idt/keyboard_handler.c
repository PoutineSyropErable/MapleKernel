#include "idt.h"
#include "keyboard_handler.h"
#include "pic.h"
#include "stdio.h"
#include "vga_terminal.h"

#define KEYBOARD_IRQ 1

void parse_scan_code(uint8_t scancode) {
}

void parse_extended_scan_code(uint8_t scancode) {
}

void keyboard_handler(uint8_t scancode) {

	bool extended_mode = false;
	static bool previous_extended_mode = false;

	bool press_not_release = false;
	if (scancode < 128) {
		press_not_release = true;
	} else if (scancode == 0xe0) {
		extended_mode = true;
	}

	if (press_not_release) {
		kprintf("scan code (press    ) = |%u:3, %h:4, %b:8|\n", scancode, scancode, scancode);

		if (previous_extended_mode) {
			kprintf("This is a special key, with an e0 prefix\n");
		}
		goto parse_scancode;
	}

	// This if is much cleaner then an else. Idk, i find this easier to read
	if (!press_not_release) {

		if (extended_mode) {
			kprintf("scan code (e0       ) = |%u:3, %h:4, %b:8|\n", scancode, scancode, scancode);
			goto parse_scancode;
		}
		// not press, not needed
		kprintf("scan code (not press) = |%u:3, %h:4, %b:8|\n", scancode, scancode, scancode);
		if (previous_extended_mode) {
			kprintf("This is a special key, with an e0 prefix\n");
		}
		terminal_putchar('\n');
	}

parse_scancode:
	if (previous_extended_mode) {
		parse_extended_scan_code(scancode);
	} else {
		// Execute the code for non special keys.
		parse_scan_code(scancode);
	}

	// const color_char_nice_t cc = {.bg = VGA_COLOR_WHITE, .fg = VGA_COLOR_RED, .c = scancode};
	// volatile color_char_nice_t* vga_text_terminal = (volatile color_char_nice_t*)VGA_MMIO_BASE;
	// vga_text_terminal[i] = cc;
	// i++;
	previous_extended_mode = extended_mode;

	PIC_sendEOI(KEYBOARD_IRQ);
}
