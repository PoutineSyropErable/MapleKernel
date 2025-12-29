#pragma once

#include <stdint.h>

// 64-bit versions (optional)
int64_t __fixsfdi(float f);      // float → int64_t
uint64_t __fixunssfdi(float f);  // float → uint64_t
float __floatdisf(int64_t i);    // int64_t → float
float __floatundisf(uint64_t i); // uint64_t → float
