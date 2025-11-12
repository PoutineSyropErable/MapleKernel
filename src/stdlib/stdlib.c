#include "stdlib.h"

// memcpy usually goes in <string.h>
void* memcpy(void* dest, const void* src, size_t n) {
	// Cast to unsigned char pointers for byte-wise copy
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;

	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	}

	return dest; // mimic standard memcpy return value
}
