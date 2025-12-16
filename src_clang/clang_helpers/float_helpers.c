#include <stdint.h>

double __extendsfdf2(float f) {
	// Simple float to double conversion
	return (double)f;
}

float __truncdfsf2(double d) {
	// Double to float conversion (if needed)
	return (float)d;
}

float __addsf3(float a, float b) { return a + b; }
float __subsf3(float a, float b) { return a - b; }
float __mulsf3(float a, float b) { return a * b; }
float __divsf3(float a, float b) { return a / b; }

int __eqsf2(float a, float b) { return (a == b) ? 0 : 1; }
int __nesf2(float a, float b) { return (a != b) ? 1 : 0; }
int __ltsf2(float a, float b) { return (a < b) ? -1 : (a > b) ? 1
	                                                          : 0; }
int __lesf2(float a, float b) { return (a <= b) ? -1 : (a > b) ? 1
	                                                           : 0; }
int __gtsf2(float a, float b) { return (a > b) ? -1 : (a < b) ? 1
	                                                          : 0; }
int __gesf2(float a, float b) { return (a >= b) ? -1 : (a < b) ? 1
	                                                           : 0; }

// Float ↔ Integer conversions
int32_t __fixsfsi(float f) {
	return (int32_t)f; // float to signed int
}

uint32_t __fixunssfsi(float f) {
	return (uint32_t)f; // float to unsigned int
}

float __floatsisf(int32_t i) {
	return (float)i; // signed int to float
}

float __floatunsisf(uint32_t i) {
	return (float)i; // unsigned int to float
}
