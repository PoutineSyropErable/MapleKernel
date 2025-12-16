// src/mod1/B.cppm
module; // Global module fragment

#include "stdio.h" // For kprintf

export module B;

import A; // B depends on A

export void greetB()
{
	greetA();
	kprintf("Hello from merged module B!\n");
}

void internalB()
{
	kprintf("Internal function in B\n");
}
