// com1.c
#include "com1.h"
#include "io.h"

#define COM1_PORT 0x3F8

static int com1_is_transmit_empty(void)
{
	return inb(COM1_PORT + 5) & 0x20;
}

void com1_putc(char c)
{
	while (com1_is_transmit_empty() == 0)
		;
	outb(COM1_PORT, c);
}

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

void com1_write(const char *str)
{
	for (; *str; str++)
	{
		if (*str == '\n')
			com1_putc('\r');
		com1_putc(*str);
	}
}

void com1_write_len(const char *str, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		if (str[i] == '\n')
			com1_putc('\r');
		com1_putc(str[i]);
	}
}
