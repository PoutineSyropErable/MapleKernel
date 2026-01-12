/* =========================================================================
 *  AUTO-GENERATED FILE — DO NOT EDIT
 *
 *  Source ELF : kernel64.elf
 * =========================================================================
 */

#pragma once
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace kernel64_size
{

/* Module size & page count */
constexpr uint64_t MODULE_SIZE = 0x000000000041a000;
constexpr uint64_t PAGE_COUNT  = (MODULE_SIZE + 0xFFF) / 0x1000;
STATIC_ASSERT(PAGE_COUNT < 0xFFFF'FFFF, "Page count must fit in a uint32_t number\n");

/* Base addresses */
constexpr uint64_t VIRTUAL_BASE		  = 0xffffffff80000000;
constexpr uint64_t KERNEL64_MAIN_ADDR = 0xffffffff80101140;

/* Module end */
constexpr uint64_t MODULE_END = 0xffffffff8041a000;

/* Text section */
constexpr uint64_t TEXT_START = 0xffffffff80000000;
constexpr uint64_t TEXT_END	  = 0xffffffff80102000;
constexpr uint64_t TEXT_SIZE  = TEXT_END - TEXT_START;

/* Text guard page */
constexpr uint64_t TEXT_GUARD_START = 0xffffffff80102000;
constexpr uint64_t TEXT_GUARD_END	= 0xffffffff80103000;
constexpr uint64_t TEXT_GUARD_SIZE	= TEXT_GUARD_END - TEXT_GUARD_START;

/* Read-only data section */
constexpr uint64_t RODATA_START = 0xffffffff80103000;
constexpr uint64_t RODATA_END	= 0xffffffff80204000;
constexpr uint64_t RODATA_SIZE	= RODATA_END - RODATA_START;

/* Rodata guard page */
constexpr uint64_t RODATA_GUARD_START = 0xffffffff80204000;
constexpr uint64_t RODATA_GUARD_END	  = 0xffffffff80204000;
constexpr uint64_t RODATA_GUARD_SIZE  = RODATA_GUARD_END - RODATA_GUARD_START;

/* Data section */
constexpr uint64_t DATA_START = 0xffffffff80204000;
constexpr uint64_t DATA_END	  = 0xffffffff80305000;
constexpr uint64_t DATA_SIZE  = DATA_END - DATA_START;

/* Data guard page */
constexpr uint64_t DATA_GUARD_START = 0xffffffff80305000;
constexpr uint64_t DATA_GUARD_END	= 0xffffffff80305000;
constexpr uint64_t DATA_GUARD_SIZE	= DATA_GUARD_END - DATA_GUARD_START;

/* BSS section */
constexpr uint64_t BSS_START = 0xffffffff80305000;
constexpr uint64_t BSS_END	 = 0xffffffff80405010;
constexpr uint64_t BSS_SIZE	 = BSS_END - BSS_START;

/* BSS guard page */
constexpr uint64_t BSS_GUARD_START = 0xffffffff80406000;
constexpr uint64_t BSS_GUARD_END   = 0xffffffff80406000;
constexpr uint64_t BSS_GUARD_SIZE  = BSS_GUARD_END - BSS_GUARD_START;

/* Stack section */
constexpr uint64_t STACK_BOTTOM = 0xffffffff80406000;
constexpr uint64_t STACK_TOP	= 0xffffffff8040a000;
constexpr uint64_t STACK_SIZE	= STACK_TOP - STACK_BOTTOM;

/* Stack guard page */
constexpr uint64_t STACK_GUARD_START = 0xffffffff8040a000;
constexpr uint64_t STACK_GUARD_END	 = 0xffffffff8040a000;
constexpr uint64_t STACK_GUARD_SIZE	 = STACK_GUARD_END - STACK_GUARD_START;

/* Heap section */
constexpr uint64_t HEAP_START = 0xffffffff8040a000;
constexpr uint64_t HEAP_END	  = 0xffffffff8041a000;
constexpr uint64_t HEAP_SIZE  = HEAP_END - HEAP_START;

/* Heap guard page */
constexpr uint64_t HEAP_GUARD_START = 0xffffffff8041a000;
constexpr uint64_t HEAP_GUARD_END	= 0xffffffff8041a000;
constexpr uint64_t HEAP_GUARD_SIZE	= HEAP_GUARD_END - HEAP_GUARD_START;

/* Total guard pages */
constexpr uint64_t TOTAL_GUARD_SIZE =
	TEXT_GUARD_SIZE + RODATA_GUARD_SIZE + DATA_GUARD_SIZE + BSS_GUARD_SIZE + STACK_GUARD_SIZE + HEAP_GUARD_SIZE;

/* Total usable memory */
constexpr uint64_t TOTAL_USABLE_SIZE = TEXT_SIZE + RODATA_SIZE + DATA_SIZE + BSS_SIZE + STACK_SIZE + HEAP_SIZE;

/* Address range helpers */
constexpr bool is_in_text(uint64_t addr)
{
	return addr >= TEXT_START && addr < TEXT_END;
}
constexpr bool is_in_rodata(uint64_t addr)
{
	return addr >= RODATA_START && addr < RODATA_END;
}
constexpr bool is_in_data(uint64_t addr)
{
	return addr >= DATA_START && addr < DATA_END;
}
constexpr bool is_in_bss(uint64_t addr)
{
	return addr >= BSS_START && addr < BSS_END;
}
constexpr bool is_in_stack(uint64_t addr)
{
	return addr >= STACK_BOTTOM && addr < STACK_TOP;
}
constexpr bool is_in_heap(uint64_t addr)
{
	return addr >= HEAP_START && addr < HEAP_END;
}
constexpr bool is_in_guard(uint64_t addr)
{
	return (addr >= TEXT_GUARD_START && addr < TEXT_GUARD_END) || (addr >= RODATA_GUARD_START && addr < RODATA_GUARD_END) ||
		   (addr >= DATA_GUARD_START && addr < DATA_GUARD_END) || (addr >= BSS_GUARD_START && addr < BSS_GUARD_END) ||
		   (addr >= STACK_GUARD_START && addr < STACK_GUARD_END) || (addr >= HEAP_GUARD_START && addr < HEAP_GUARD_END);
}

/* Section order verification */
constexpr bool assert_section_order(uint64_t prev_end, uint64_t next_start)
{
	return prev_end == next_start;
}

/* Memory region types */
enum class region_t : uint8_t
{
	TEXT,
	RODATA,
	DATA,
	BSS,
	STACK,
	HEAP,
	GUARD,
	INVALID
};

/* Helper function to determine region from address */
constexpr region_t get_region_type(uint64_t addr)
{
	return is_in_text(addr)		? region_t::TEXT
		   : is_in_rodata(addr) ? region_t::RODATA
		   : is_in_data(addr)	? region_t::DATA
		   : is_in_bss(addr)	? region_t::BSS
		   : is_in_stack(addr)	? region_t::STACK
		   : is_in_heap(addr)	? region_t::HEAP
		   : is_in_guard(addr)	? region_t::GUARD
								: region_t::INVALID;
}

constexpr const char *region_to_string(region_t r)
{
	switch (r)
	{
	case region_t::TEXT: return "TEXT";
	case region_t::RODATA: return "RODATA";
	case region_t::DATA: return "DATA";
	case region_t::BSS: return "BSS";
	case region_t::STACK: return "STACK";
	case region_t::HEAP: return "HEAP";
	case region_t::GUARD: return "GUARD";
	case region_t::INVALID: return "INVALID";
	}
	return "UNKNOWN"; // fallback (unreachable if all enum values covered)
}

} // namespace kernel64_size
