#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
	__asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

#define COM1 0x3F8

void serial_init() {
	outb(COM1 + 1, 0x00); // Disable interrupts
	outb(COM1 + 3, 0x80); // Enable DLAB
	outb(COM1 + 0, 0x03); // Set baud rate to 38400 (low byte)
	outb(COM1 + 1, 0x00); //                  (high byte)
	outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
	outb(COM1 + 2, 0xC7); // Enable FIFO, clear them
	outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static inline int serial_is_transmit_ready() {
	return inb(COM1 + 5) & 0x20;
}

void serial_write(char c) {
	while (!serial_is_transmit_ready())
		;
	outb(COM1, c);
}

void print_serial(const char* s) {
	while (*s)
		serial_write(*s++);
}

void kernel_main(void) {
	// serial_init();
	print_serial("\n\n---Hello from serial kernel!\n");

	while (1)
		__asm__ __volatile__("hlt");
}
