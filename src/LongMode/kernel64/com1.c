// com1.c
#include "com1.h"
#include "io.h"

#define COM1_PORT 0x3F8

void com1_init(void)
{
	// Disable interrupts
	outb(COM1_PORT + 1, 0x00);

	// Set baud rate (115200)
	outb(COM1_PORT + 3, 0x80); // Enable DLAB
	outb(COM1_PORT + 0, 0x01); // Divisor low byte (115200 baud)
	outb(COM1_PORT + 1, 0x00); // Divisor high byte

	// 8 bits, no parity, one stop bit
	outb(COM1_PORT + 3, 0x03);

	// Enable FIFO, clear them, with 14-byte threshold
	outb(COM1_PORT + 2, 0xC7);

	// Enable interrupts if desired
	// outb(COM1_PORT + 4, 0x0B);
}

static int com1_is_transmit_empty(void)
{
	return inb(COM1_PORT + 5) & 0x20;
	// 0x20 = 32 = 1 0___0__0__0   0000
	//               128 64 32 16  8421
	// 0x20 = 0010 0000
	//        7654  3210
	// Bit 5
}

void com1_putc(char c)
{
	while (com1_is_transmit_empty() == 0)
		;
	outb(COM1_PORT, c);
}

uint64_t com1_write_c_MANGLED(const char *str)
{
	uint64_t i = 0;
	while (str[i])
	{
		com1_putc(str[i]);
		i++;
	}
	return i;
}

void com1_write_len_c_MANGLED(const char *str, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		com1_putc(str[i]);
	}
}

#define COM1 0x3F8
void serial_write_char_MANGLED(char c)
{

	// Wait until the transmit buffer is empty
	while (!(inb(COM1 + 5) & 0x20))
		;
	outb(COM1, c);
}

void serial_write_string_MANGLED(const char *str)
{
	while (*str)
	{
		serial_write_char_MANGLED(*str++);
	}
}
