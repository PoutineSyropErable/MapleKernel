#include "com1.h"
#include "framebuffer_shared.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void *memmove(void *dest, const void *src, size_t n)
{
	unsigned char		*d = (unsigned char *)dest;
	const unsigned char *s = (const unsigned char *)src;

	if (d == s || n == 0)
		return dest;

	if (d < s)
	{
		// Copy forward (no overlap or src before dest)
		for (size_t i = 0; i < n; i++)
		{
			d[i] = s[i];
		}
	}
	else
	{
		// Copy backward (dest before src, overlapping)
		for (size_t i = n; i > 0; i--)
		{
			d[i - 1] = s[i - 1];
		}
	}
	return dest;
}

// Also implement these - they'll likely be needed soon
void *memcpy(void *dest, const void *src, size_t n)
{
	// memmove handles all cases correctly
	return memmove(dest, src, n);
}

void *memset(void *dest, int value, size_t n)
{
	unsigned char *d   = (unsigned char *)dest;
	unsigned char  val = (unsigned char)value;

	for (size_t i = 0; i < n; i++)
	{
		d[i] = val;
	}

	return dest; // mimic standard memset return value
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
	com1_write_c_MANGLED(x);
	com1_write_c_MANGLED("Com1 write ro data\n");

	fill_screen(0x000000);

	com1_write_c_MANGLED("Going to Zig! No More C\n");

	kernel64_zig_main();
}
