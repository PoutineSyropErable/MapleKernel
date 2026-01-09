#include "com1.h"
#include "framebuffer_shared.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n)
{
	char	   *d = (char *)dest;
	const char *s = (const char *)src;

	for (size_t i = 0; i < n; i++)
	{
		d[i] = s[i];
	}

	return dest;
}

__attribute__((noinline)) void fill_screen(volatile uint32_t color)
{

	volatile uint32_t *fb_ptr = (volatile uint32_t *)FB_MMIO_BASE;
	volatile uint32_t *fb_end = fb_ptr + 1920 * 1080;
	for (volatile uint32_t *mmio_ptr = fb_ptr; mmio_ptr < fb_end; mmio_ptr++)
	{
		*mmio_ptr = color;
	}
}

void kernel64_main()
{
	char *str = "Com1 write ro data\n";
	char  c	  = *(char *)0xffffffff80003001;
	// char	 c2	 = *str;
	uint64_t i = com1_write(str);
	fill_screen(c);
	if (false)
	{

		const char x[] = "\n\n=============Hello from 64 bit================\n\n";
		com1_init();
		com1_write(x);
		com1_write("Com1 write ro data\n");

		const char y[] = "\n\n=============Hello from 64 bit (serial write string)================\n\n";
		serial_write_string(y);
		serial_write_string("Serial write string ro data\n");

		const char *z	   = "Test 123";
		const char	zz[20] = {0};
		memcpy((void *)zz, z, 20);
		com1_write(zz);
		com1_write_len(z, 10);
		com1_putc(z[0]);
		com1_putc(z[1]);
		com1_putc(z[2]);

		// char third_char = z[2]; // Reading from .rodata SUCCEEDS
		// com1_putc(third_char);

		fill_screen(z[2]);
	}
}
