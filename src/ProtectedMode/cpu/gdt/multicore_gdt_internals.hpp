#pragma once
#include <stdint.h>

#include "multicore.h"

namespace multicore_gdt
{

#define NEW_GDT_ENTRY_COUNT (4 + 2 * (MAX_CORE_COUNT - 1))

} // namespace multicore_gdt
