#pragma once
#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{
#endif

	struct fpu_features
	{
		bool fpu; // x87 FPU
		bool mmx;
		bool sse;
		bool sse2;
		bool sse3;
		bool ssse3;
		bool sse41;
		bool sse42;
		bool avx;
		bool xsave; // needed for AVX
	};

	struct fpu_features init_fpu(void);

#ifdef __cplusplus
}
#endif
