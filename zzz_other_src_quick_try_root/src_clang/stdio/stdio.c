#include "stdio.h"
#include <stdint.h>

// COM1 I/O port
#define COM1_PORT 0x3F8

// Minimal I/O functions (x86)
static inline void outb(uint16_t port, uint8_t val) {
	asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

// Wait until serial port is ready to send
static int serial_is_transmit_empty() {
	return (inb(COM1_PORT + 5) & 0x20) != 0;
}

// Write a single byte to COM1
static void serial_write_char(char c) {
	while (!serial_is_transmit_empty())
		;
	outb(COM1_PORT, c);
}

// Write a string to COM1
void serial_write_string(const char* str) {
	while (*str) {
		serial_write_char(*str++);
	}
}

// Convert uint32_t to hexadecimal and print
void serial_write_hex(uint32_t value) {
	static const char hex_digits[] = "0123456789ABCDEF";
	serial_write_string("0x");

	for (int i = 7; i >= 0; i--) {
		uint8_t nibble = (value >> (i * 4)) & 0xF;
		serial_write_char(hex_digits[nibble]);
	}
}

void serial_write_uint(uint32_t value) {
	char buffer[10]; // max digits for 32-bit uint32_t
	int i = 0;

	if (value == 0) {
		serial_write_char('0');
		return;
	}

	// Convert to string in reverse
	while (value > 0) {
		buffer[i++] = '0' + (value % 10);
		value /= 10;
	}

	// Print in correct order
	for (int j = i - 1; j >= 0; j--) {
		serial_write_char(buffer[j]);
	}
}

// Write int32_t as decimal (signed)
static void serial_write_int(int32_t value) {
	if (value < 0) {
		serial_write_char('-');
		value = -value;
	}
	serial_write_uint((uint32_t)value);
}

// Super simple float printer (bad but works)
static void serial_write_float(float f) {
	// Handle sign
	serial_write_string("got here 0\n");
	if (f < 0) {
		serial_write_char('-');
		f = -f;
	}
	serial_write_string("got here 1\n");

	// Integer part
	int32_t int_part = (int32_t)f;
	serial_write_int(int_part);
	serial_write_string("got here 2\n");

	// Decimal point
	serial_write_char('.');

	// Fraction part (2 decimal places)
	float frac = f - (float)int_part;
	for (int i = 0; i < 2; i++) {
		frac *= 10.0f;
		int digit = (int)frac;
		serial_write_char('0' + digit);
		frac -= (float)digit;
	}
}

// Example printf for one uint32_t
void serial_printf(const char* prefix, uint32_t value, bool decNotHex) {
	serial_write_string(prefix);
	if (decNotHex) {

		serial_write_uint(value);
	} else {

		serial_write_hex(value);
	}
	serial_write_string("\n");
}

// BAD AND MINIMAL PRINTF
void kprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	while (*fmt) {
		if (*fmt == '%') {
			fmt++; // Skip %

			switch (*fmt) {
			case 'u': { // %u - unsigned int
				uint32_t val = va_arg(args, uint32_t);
				serial_write_uint(val);
				break;
			}
			case 'd': { // %d - signed int
				int32_t val = va_arg(args, int32_t);
				serial_write_int(val);
				break;
			}
			case 'x': { // %x - hex
				uint32_t val = va_arg(args, uint32_t);
				serial_write_hex(val);
				break;
			}
			case 'f': { // %f - float (promoted to double)
				serial_write_string("Chose %f\n");
				double val = va_arg(args, double);
				serial_write_string("got here\n");
				float r = (float)val;
				serial_write_string("converted\n");
				serial_write_float(r); // Cast back to float
				serial_write_string("finished function\n");
				break;
			}
			case 'c': {                             // %c - char
				char val = (char)va_arg(args, int); // chars promoted to int
				serial_write_char(val);
				break;
			}
			case 's': { // %s - string
				char* val = va_arg(args, char*);
				serial_write_string(val);
				break;
			}
			case '%': { // %% - literal %
				serial_write_char('%');
				break;
			}
			default: { // Unknown format, print as-is
				serial_write_char('%');
				serial_write_char(*fmt);
				break;
			}
			}
			fmt++; // Move past format specifier
		} else {
			serial_write_char(*fmt);
			fmt++;
		}
	}

	va_end(args);
}
