#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#define COM1 0x3F8

// Port I/O helpers (x86-64 inline assembly)
static inline void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

// Wait until the transmit buffer is empty
static inline void wait_tx() {
	while ((inb(COM1 + 5) & 0x20) == 0)
		;
}

// Write a byte to COM1
static inline void write_com1(uint8_t c) {
	wait_tx();
	outb(COM1, c);
}

#define write_char(str) write_com1(str)

// C-style write string
void write_string_c(const char* str) {
	while (*str) {
		write_com1(*str);
		str++;
	}
}
