#include "assert.h"
#include "math.h"

double sqrt_quake32(double x)
{
#ifdef DEBUG
	assert(sizeof(double) == 4, "Size must be 4");
#endif

	union du_uts
	{
		double	 d;
		uint32_t u;
	} x_uts	   = {.d = x};
	uint32_t u = x_uts.u;

	abort_msg("Not Implemented yet!\n");
	return 1.f;
}

double sqrt(double x)
{
	double result;
	__asm__ volatile("fldl %1\n\t" // load x onto FPU stack
					 "fsqrt\n\t"   // compute sqrt
					 "fstpl %0"	   // store result into memory
		: "=m"(result)			   // output
		: "m"(x)				   // input
	);
	return result;
}
