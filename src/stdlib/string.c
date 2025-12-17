#include "assert.h"
#include "string.h"

// memcpy usually goes in <string.h>
void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	// Cast to unsigned char pointers for byte-wise copy
	unsigned char		*d = (unsigned char *)dest;
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

// memset usually goes in <string.h>
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
