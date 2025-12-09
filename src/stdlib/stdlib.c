#include "assert.h"
#include "stdlib.h"
#include <stdint.h>

// memcpy usually goes in <string.h>
void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    // Cast to unsigned char pointers for byte-wise copy
    unsigned char       *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

#ifdef DEBUG
    // Check for overlap in debug mode
    if ((d > s && d < s + n) || (s > d && s < d + n))
    {
        abort_msg("Overlapping memcopy!\n");
    }
#endif

    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }

    return dest; // mimic standard memcpy return value
}

// Single struct definition using uintptr_t
struct stackframe
{
    struct stackframe *bp; // base pointer (EBP/RBP)
    uintptr_t          ip; // instruction pointer (EIP/RIP)
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

/**
 * strcmp - Compare two strings
 * @s1: First string
 * @s2: Second string
 *
 * Returns:
 *  <0 if s1 < s2
 *   0 if s1 == s2
 *  >0 if s1 > s2
 *
 * Comparison is based on unsigned character values.
 */
int strcmp(const char *s1, const char *s2)
{
    // Cast to unsigned char for proper comparison
    // (char may be signed, we want 0-255 range)
    const unsigned char *us1 = (const unsigned char *)s1;
    const unsigned char *us2 = (const unsigned char *)s2;

    // Compare until difference found or end of string
    while (*us1 && *us1 == *us2)
    {
        us1++;
        us2++;
    }

    // Return the difference at the point of mismatch
    // This gives correct ordering based on ASCII values
    return (int)*us1 - (int)*us2;
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
