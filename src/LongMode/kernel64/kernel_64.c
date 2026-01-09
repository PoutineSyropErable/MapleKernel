#include "com1.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void kernel64_main()
{
	const char x[] = "\n\n=============Hello from 64 bit================\n\n";
	com1_write(x);
	com1_write("Hello\n");
}
