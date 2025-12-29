// float_helpers.h
#ifndef FLOAT_HELPERS_H
#define FLOAT_HELPERS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================
// FLOAT ↔ DOUBLE CONVERSIONS
// ============================================

/**
 * Convert float to double (float → double)
 * Called by Clang when passing float to variadic functions
 */
double __extendsfdf2(float f);

/**
 * Convert double to float (double → float)
 */
float __truncdfsf2(double d);

// ============================================
// FLOAT ARITHMETIC
// ============================================

float __addsf3(float a, float b); // float addition
float __subsf3(float a, float b); // float subtraction
float __mulsf3(float a, float b); // float multiplication
float __divsf3(float a, float b); // float division

// ============================================
// FLOAT COMPARISONS
// ============================================
// Returns: -1 (a < b), 0 (a == b), 1 (a > b)

int __eqsf2(float a, float b); // a == b ? 0 : 1
int __nesf2(float a, float b); // a != b ? 1 : 0
int __ltsf2(float a, float b); // a < b ? -1 : (a > b ? 1 : 0)
int __lesf2(float a, float b); // a <= b ? -1 : (a > b ? 1 : 0)
int __gtsf2(float a, float b); // a > b ? -1 : (a < b ? 1 : 0)
int __gesf2(float a, float b); // a >= b ? -1 : (a < b ? 1 : 0)

// ============================================
// DOUBLE ARITHMETIC (if needed)
// ============================================

double __adddf3(double a, double b);
double __subdf3(double a, double b);
double __muldf3(double a, double b);
double __divdf3(double a, double b);

// ============================================
// DOUBLE COMPARISONS (if needed)
// ============================================

int __eqdf2(double a, double b);
int __nedf2(double a, double b);
int __ltdf2(double a, double b);
int __ledf2(double a, double b);
int __gtdf2(double a, double b);
int __gedf2(double a, double b);

// ============================================
// FLOAT ↔ INTEGER CONVERSIONS
// ============================================

int32_t __fixsfsi(float f);      // float → int32_t
uint32_t __fixunssfsi(float f);  // float → uint32_t
float __floatsisf(int32_t i);    // int32_t → float
float __floatunsisf(uint32_t i); // uint32_t → float

#ifdef __cplusplus
}
#endif

#endif // FLOAT_HELPERS_H
