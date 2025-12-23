#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * memset - Fill a block of memory with a specified value
	 * @dest: Pointer to the memory block to fill
	 * @value: Value to set (converted to unsigned char)
	 * @n: Number of bytes to fill
	 *
	 * Returns:
	 *  Pointer to the memory block @dest.
	 *
	 * The memory at @dest is set byte-by-byte to @value for @n bytes.
	 * In DEBUG mode, overlapping issues may be checked.
	 */
	void *memset(void *dest, int value, size_t n);

	/**
	 * memcpy - Copy a block of memory from source to destination
	 * @dest: Pointer to the destination memory block
	 * @src: Pointer to the source memory block
	 * @n: Number of bytes to copy
	 *
	 * Returns:
	 *  Pointer to the destination memory block @dest.
	 *
	 * Copies @n bytes from @src to @dest. The memory areas must not overlap.
	 * In DEBUG mode, overlapping memory will trigger an abort.
	 */
	void *memcpy(void *dest, const void *src, size_t n);

	void *memcpy32(void *dest, const void *src, size_t n);

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
	int strcmp(const char *s1, const char *s2);

	int strncmp(const char *s1, const char *s2, size_t len);

	int find_string_offset(const char *haystack, int haystack_len, const char *needle);

#ifdef __cplusplus
}
#endif
