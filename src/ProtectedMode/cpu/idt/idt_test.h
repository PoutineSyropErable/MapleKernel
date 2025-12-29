#pragma once
#include "stdio.h"

// Define all interrupt vectors you want to test
#define INT_VECTOR_LIST                                                                                                                    \
	X(0)                                                                                                                                   \
	X(1)                                                                                                                                   \
	X(2)                                                                                                                                   \
	X(3)                                                                                                                                   \
	X(4)                                                                                                                                   \
	X(5)                                                                                                                                   \
	X(6)                                                                                                                                   \
	X(7)                                                                                                                                   \
	X(8)                                                                                                                                   \
	X(9)                                                                                                                                   \
	X(10)                                                                                                                                  \
	X(11)                                                                                                                                  \
	X(12)                                                                                                                                  \
	X(13)                                                                                                                                  \
	X(14)                                                                                                                                  \
	X(15)                                                                                                                                  \
	X(16)                                                                                                                                  \
	X(17)                                                                                                                                  \
	X(18)                                                                                                                                  \
	X(19)                                                                                                                                  \
	X(20)                                                                                                                                  \
	X(21)                                                                                                                                  \
	X(22)                                                                                                                                  \
	X(23)                                                                                                                                  \
	X(24)                                                                                                                                  \
	X(25)                                                                                                                                  \
	X(26)                                                                                                                                  \
	X(27)                                                                                                                                  \
	X(28)                                                                                                                                  \
	X(29)                                                                                                                                  \
	X(30)                                                                                                                                  \
	X(31)

// Generate a test function for each vector
#define X(vec)                                                                                                                             \
	static inline void test_int_##vec(void)                                                                                                \
	{                                                                                                                                      \
		kprintf("Calling interrupt handler %u\n", vec);                                                                                    \
		__asm__ volatile("int $" #vec : : : "memory");                                                                                     \
	}
INT_VECTOR_LIST
#undef X

// Generate a function that calls all of them
static inline void test_all_ints(void)
{
#define X(vec) test_int_##vec();
	INT_VECTOR_LIST
#undef X
}
