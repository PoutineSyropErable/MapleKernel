// src/mod1/A.cppm
module; // Global module fragment for legacy headers

#include "stdio.h" // For kprintf

export module A; // Module declaration

export void greetA()
{
	kprintf("Hello from module A!\n");
}

void internalA()
{
	kprintf("Internal function in A\n");
}
