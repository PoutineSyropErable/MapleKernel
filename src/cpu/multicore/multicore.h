#pragma once
#include "gdt.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_CORE_COUNT 8
#define NEW_GDT_ENTRY_COUNT (4 + 2 * MAX_CORE_COUNT)

	extern volatile bool core_has_booted[MAX_CORE_COUNT];			 // [i = reciever][ j = sender]
	extern volatile bool master_tells_core_to_start[MAX_CORE_COUNT]; // [i = reciever][ j = sender]
	extern void (*core_mains[8])();
	extern volatile uint8_t last_interrupt_received[MAX_CORE_COUNT][MAX_CORE_COUNT]; // [i = reciever][ j = sender]
	extern uint8_t			runtime_core_count;

#define INTERRUPT_ENTERED_MAIN 41
#define NO_INTERRUPT 255

	extern void core_bootstrap();
	extern void application_core_main();

	__attribute__((section(".bss.multicore_gdt16"))) extern "C" GDT_ENTRY new_gdt[4 + 2 * MAX_CORE_COUNT];

#ifdef __cplusplus
}
#endif
