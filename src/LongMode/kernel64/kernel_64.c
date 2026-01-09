#include "com1.h"
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

void kernel64_main()
{
	const char x[] = "\n\n=============Hello from 64 bit================\n\n";
	com1_write(x);
	com1_write("Com1 write ro data\n");

	const char y[] = "\n\n=============Hello from 64 bit (serial write string)================\n\n";
	serial_write_string(y);
	serial_write_string("Serial write string ro data\n");

	const char *z = "Test 123";
	const char	zz[20];
	memcpy((void *)zz, z, 20);
	com1_putc(z[0] + 48);
}
