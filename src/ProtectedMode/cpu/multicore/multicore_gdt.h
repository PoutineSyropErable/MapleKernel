#pragma once
#include "gdt.h"
#include "multicore.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	__attribute__((aligned(8), section(".bss.multicore_gdt16"))) extern GDT_ENTRY new_gdt[4 + 2 * (MAX_CORE_COUNT - 1)];

#ifdef __cplusplus
}
#endif
