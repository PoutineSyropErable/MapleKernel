#include <stdint.h>

// Float ↔ Long conversions (64-bit, optional)
int64_t __fixsfdi(float f) {
	return (int64_t)f;
}

uint64_t __fixunssfdi(float f) {
	return (uint64_t)f;
}

float __floatdisf(int64_t i) {
	return (float)i;
}

float __floatundisf(uint64_t i) {
	return (float)i;
}
