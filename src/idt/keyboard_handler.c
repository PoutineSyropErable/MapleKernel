#include "idt.h"
#include "keyboard_handler.h"
#include "pic.h"
#include "stdio.h"
#include "vga_terminal.h"

#define KEYBOARD_IRQ 1

void keyboard_handler(uint8_t scancode) {

	static bool press = true;
	kprintf("|%u, %h, %b|\n", scancode, scancode, scancode);
	if (!press) {
		terminal_putchar('\n');
	}
	press = !press;

	PIC_sendEOI(KEYBOARD_IRQ);
}
