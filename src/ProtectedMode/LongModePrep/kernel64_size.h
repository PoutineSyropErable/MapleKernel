/* =========================================================================
 *  AUTO-GENERATED FILE — DO NOT EDIT
 *
 *  Source ELF : kernel64.elf
 *  Symbol     : __module_size
 * =========================================================================
 */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KERNEL64_MODULE_SIZE 0x000000000041d000
#define KERNEL64_PAGE_COUNT ((KERNEL64_MODULE_SIZE + 0xFFF) / 0x1000)

/* Base addresses */
#define KERNEL64_VIRTUAL_BASE 0xffffffff80000000

/* Module boundaries */
#define KERNEL64_MODULE_END 0xffffffff8041d000

/* Text section */
#define KERNEL64_TEXT_START 0xffffffff80000000
#define KERNEL64_TEXT_END 0xffffffff8010104a
#define KERNEL64_TEXT_SIZE (KERNEL64_TEXT_END - KERNEL64_TEXT_START)

/* Text guard page */
#define KERNEL64_TEXT_GUARD_START 0xffffffff80102000
#define KERNEL64_TEXT_GUARD_END 0xffffffff80103000
#define KERNEL64_TEXT_GUARD_SIZE (KERNEL64_TEXT_GUARD_END - KERNEL64_TEXT_GUARD_START)

/* Read-only data section */
#define KERNEL64_RODATA_START 0xffffffff80103000
#define KERNEL64_RODATA_END 0xffffffff80203040
#define KERNEL64_RODATA_SIZE (KERNEL64_RODATA_END - KERNEL64_RODATA_START)

/* Rodata guard page */
#define KERNEL64_RODATA_GUARD_START 0xffffffff80204000
#define KERNEL64_RODATA_GUARD_END 0xffffffff80205000
#define KERNEL64_RODATA_GUARD_SIZE (KERNEL64_RODATA_GUARD_END - KERNEL64_RODATA_GUARD_START)

/* Data section */
#define KERNEL64_DATA_START 0xffffffff80205000
#define KERNEL64_DATA_END 0xffffffff80305000
#define KERNEL64_DATA_SIZE (KERNEL64_DATA_END - KERNEL64_DATA_START)

/* Data guard page */
#define KERNEL64_DATA_GUARD_START 0xffffffff80305000
#define KERNEL64_DATA_GUARD_END 0xffffffff80306000
#define KERNEL64_DATA_GUARD_SIZE (KERNEL64_DATA_GUARD_END - KERNEL64_DATA_GUARD_START)

/* BSS section */
#define KERNEL64_BSS_START 0xffffffff80306000
#define KERNEL64_BSS_END 0xffffffff80406000
#define KERNEL64_BSS_SIZE (KERNEL64_BSS_END - KERNEL64_BSS_START)

/* BSS guard page */
#define KERNEL64_BSS_GUARD_START 0xffffffff80406000
#define KERNEL64_BSS_GUARD_END 0xffffffff80407000
#define KERNEL64_BSS_GUARD_SIZE (KERNEL64_BSS_GUARD_END - KERNEL64_BSS_GUARD_START)

/* Stack section */
#define KERNEL64_STACK_BOTTOM 0xffffffff80407000
#define KERNEL64_STACK_TOP 0xffffffff8040b000
#define KERNEL64_STACK_SIZE (KERNEL64_STACK_TOP - KERNEL64_STACK_BOTTOM)

/* Stack guard page */
#define KERNEL64_STACK_GUARD_START 0xffffffff8040b000
#define KERNEL64_STACK_GUARD_END 0xffffffff8040c000
#define KERNEL64_STACK_GUARD_SIZE (KERNEL64_STACK_GUARD_END - KERNEL64_STACK_GUARD_START)

/* Heap section */
#define KERNEL64_HEAP_START 0xffffffff8040c000
#define KERNEL64_HEAP_END 0xffffffff8041c000
#define KERNEL64_HEAP_SIZE (KERNEL64_HEAP_END - KERNEL64_HEAP_START)

/* Heap guard page */
#define KERNEL64_HEAP_GUARD_START 0xffffffff8041c000
#define KERNEL64_HEAP_GUARD_END 0xffffffff8041d000
#define KERNEL64_HEAP_GUARD_SIZE (KERNEL64_HEAP_GUARD_END - KERNEL64_HEAP_GUARD_START)

/* Section order verification macros */
#define ASSERT_SECTION_ORDER(prev_end, next_start) ((prev_end) == (next_start) ? 0 : 1)

/* Check if address is within guard page (for debugging) */
#define IS_IN_GUARD_PAGE(addr)                                                                                                             \
	(((addr) >= KERNEL64_TEXT_GUARD_START && (addr) < KERNEL64_TEXT_GUARD_END) ||                                                          \
		((addr) >= KERNEL64_RODATA_GUARD_START && (addr) < KERNEL64_RODATA_GUARD_END) ||                                                   \
		((addr) >= KERNEL64_DATA_GUARD_START && (addr) < KERNEL64_DATA_GUARD_END) ||                                                       \
		((addr) >= KERNEL64_BSS_GUARD_START && (addr) < KERNEL64_BSS_GUARD_END) ||                                                         \
		((addr) >= KERNEL64_STACK_GUARD_START && (addr) < KERNEL64_STACK_GUARD_END) ||                                                     \
		((addr) >= KERNEL64_HEAP_GUARD_START && (addr) < KERNEL64_HEAP_GUARD_END))

/* Total guard pages size */
#define KERNEL64_TOTAL_GUARD_SIZE                                                                                                          \
	(KERNEL64_TEXT_GUARD_SIZE + KERNEL64_RODATA_GUARD_SIZE + KERNEL64_DATA_GUARD_SIZE + KERNEL64_BSS_GUARD_SIZE +                          \
		KERNEL64_STACK_GUARD_SIZE + KERNEL64_HEAP_GUARD_SIZE)

/* Total usable memory (excluding guard pages) */
#define KERNEL64_TOTAL_USABLE_SIZE                                                                                                         \
	(KERNEL64_TEXT_SIZE + KERNEL64_RODATA_SIZE + KERNEL64_DATA_SIZE + KERNEL64_BSS_SIZE + KERNEL64_STACK_SIZE + KERNEL64_HEAP_SIZE)

/* Convenience macros for section checking */
#define IS_IN_TEXT_SECTION(addr) ((addr) >= KERNEL64_TEXT_START && (addr) < KERNEL64_TEXT_END)

#define IS_IN_RODATA_SECTION(addr) ((addr) >= KERNEL64_RODATA_START && (addr) < KERNEL64_RODATA_END)

#define IS_IN_DATA_SECTION(addr) ((addr) >= KERNEL64_DATA_START && (addr) < KERNEL64_DATA_END)

#define IS_IN_BSS_SECTION(addr) ((addr) >= KERNEL64_BSS_START && (addr) < KERNEL64_BSS_END)

#define IS_IN_STACK_SECTION(addr) ((addr) >= KERNEL64_STACK_BOTTOM && (addr) < KERNEL64_STACK_TOP)

#define IS_IN_HEAP_SECTION(addr) ((addr) >= KERNEL64_HEAP_START && (addr) < KERNEL64_HEAP_END)

/* Memory region types for page table setup */
typedef enum
{
	KERNEL64_REGION_TEXT,
	KERNEL64_REGION_RODATA,
	KERNEL64_REGION_DATA,
	KERNEL64_REGION_BSS,
	KERNEL64_REGION_STACK,
	KERNEL64_REGION_HEAP,
	KERNEL64_REGION_GUARD,
	KERNEL64_REGION_INVALID
} kernel64_region_t;

/* Helper function to determine region type from address */
static inline kernel64_region_t kernel64_get_region_type(uintptr_t addr)
{
	if (IS_IN_TEXT_SECTION(addr))
		return KERNEL64_REGION_TEXT;
	if (IS_IN_RODATA_SECTION(addr))
		return KERNEL64_REGION_RODATA;
	if (IS_IN_DATA_SECTION(addr))
		return KERNEL64_REGION_DATA;
	if (IS_IN_BSS_SECTION(addr))
		return KERNEL64_REGION_BSS;
	if (IS_IN_STACK_SECTION(addr))
		return KERNEL64_REGION_STACK;
	if (IS_IN_HEAP_SECTION(addr))
		return KERNEL64_REGION_HEAP;
	if (IS_IN_GUARD_PAGE(addr))
		return KERNEL64_REGION_GUARD;
	return KERNEL64_REGION_INVALID;
}
