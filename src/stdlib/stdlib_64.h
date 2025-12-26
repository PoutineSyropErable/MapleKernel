#pragma once
#include <stddef.h>
#include <stdint.h>

static inline void *alloca(size_t size)
{
	void *ptr;
	asm volatile("subq %1, %%rsp\n" // move stack pointer down
				 "mov %%rsp, %0\n"	// return new stack pointer
		: "=r"(ptr)					// output
		: "r"(size)					// input
		: "rsp"						// clobbered
	);
	return ptr;
}
