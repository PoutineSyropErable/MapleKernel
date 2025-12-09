#pragma once
#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n);

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

int find_string_offset(const char *haystack, int haystack_len, const char *needle);
