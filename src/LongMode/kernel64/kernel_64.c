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

void kernel64_zig_main();

void kernel64_main()
{
		com1_init();


		const char x[] = "\n\n=============Hello from 64 bit================\n\n";
		com1_write(x);
		com1_write("Com1 write ro data\n");

		fill_screen(0x000000);

		com1_write("Going to Zig! No More C\n");

		kernel64_zig_main();
		

}
