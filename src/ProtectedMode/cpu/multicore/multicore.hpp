#pragma once
#include "multicore.h"
#include <stdbool.h>
#include <stdint.h>

namespace multicore
{

extern volatile bool entered_main[MAX_CORE_COUNT];
extern volatile bool acknowledged_entered_main[MAX_CORE_COUNT];

} // namespace multicore
