#pragma once
#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{
#endif
	// ================ Start

#define PIT_FREQ_HZ 1'193'182u

	int	 wait(float seconds);
	void wait_test();

	// =============== End
#ifdef __cplusplus
}
#endif
