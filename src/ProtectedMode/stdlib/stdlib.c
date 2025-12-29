#include "assert.h"
#include "stdlib.h"
#include "string.h"
#include <stdint.h>

// Single struct definition using uintptr_t
struct stackframe
{
	struct stackframe *bp; // base pointer (EBP/RBP)
	uintptr_t		   ip; // instruction pointer (EIP/RIP)
};

void print_stack_trace(void)
{
	struct stackframe *frame;

#ifdef __x86_64__
	__asm__ volatile("movq %%rbp, %0" : "=r"(frame));
#else
	__asm__ volatile("mov %%ebp, %0" : "=r"(frame));
#endif

	kprintf("Stack trace:\n");

	for (int i = 0; frame && i < 16; i++)
	{
		kprintf("	i=#%d: rbp=%h\n", i, (uintptr_t)frame->ip);

		// Check for invalid frame pointer
		if ((uintptr_t)frame->bp <= (uintptr_t)frame)
		{
			break;
		}

		frame = frame->bp;
	}
}

int find_string_offset(const char *haystack, int haystack_len, const char *needle)
{
	if (!haystack || !needle || haystack_len <= 0)
		return -1;

	int needle_len = 0;
	// Calculate needle length (could also use strlen if available)
	for (const char *p = needle; *p != '\0'; p++)
	{
		needle_len++;
		// Safety: prevent infinite loop if needle isn't null-terminated
		if (needle_len > 4096)
			return -1;
	}

	if (needle_len == 0)
		return 0; // Empty string is always at offset 0

	// Search for the string
	for (int i = 0; i <= haystack_len - needle_len; i++)
	{
		int match = 1;
		for (int j = 0; j < needle_len; j++)
		{
			if (haystack[i + j] != needle[j])
			{
				match = 0;
				break;
			}
		}
		if (match)
		{
			return i;
		}
	}

	return -1;
}
