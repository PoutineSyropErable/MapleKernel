// src/mod1/C.cpp
module; // Global module fragment

#include "stdio.h" // For kprintf

module C;

void greetC()
{
	kprintf("Hello from module C!\n");
}

int add(int a, int b)
{
	return a + b;
}
