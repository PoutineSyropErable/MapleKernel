#pragma once
#include "gdt.h"
#include "multicore.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	__attribute__((aligned(8), section(".bss.multicore_gdt16"))) extern GDT_ENTRY new_gdt[NEW_GDT_ENTRY_COUNT];

#ifdef __cplusplus
}
#endif
