#include "intrinsics.h"
#include "pit.h"
#include "pit.hpp"
// ===================== Start of CPP stuff
using namespace pit;

// ===================== End of Cpp Stuff

int pit::wait(float seconds)
{
    for (uint32_t i = 0; i < seconds * 1000 * 1000 * 25; i++)
    {
	__nop();
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
{
#endif
    // ================ Start of C stuff
    int wait(float seconds)
    {
	return pit::wait(seconds);
    }

// =============== End of C stuff
#ifdef __cplusplus
}
#endif
