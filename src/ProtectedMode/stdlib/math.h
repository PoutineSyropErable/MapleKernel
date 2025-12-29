#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	// Need sse support, both for the compiler, and it needs sse to be enabled at runtime in qemu
	static inline double sqrt_fast(double x)
	{
		double result;
		__asm__ volatile("sqrtsd %1, %0" : "=x"(result) : "x"(x));
		return result;
	}

	double sqrt_quake32(double x);

	double sqrt(double x);

#ifdef __cplusplus
}
#endif
