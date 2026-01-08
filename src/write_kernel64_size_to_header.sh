#!/usr/bin/env bash

set -eou pipefail

BUILD32_DIR="../build32"
BUILD64_DIR="../build64"

LONG_MODE_PREP="./ProtectedMode/LongModePrep/"
OUTPUT_HEADER_FILE="$LONG_MODE_PREP/kernel64_size.h"

KERNEL64_ELF="$BUILD64_DIR/kernel64.elf"

if [[ ! -f "$KERNEL64_ELF" ]]; then
	printf -- "\n\n========= The kernel 64 elf: %s is not a file =======\n\n" "$KERNEL64_ELF"
	exit 1
fi

function get_symbol() {
	symbol_name="$1"
	local value

	value=$(readelf -Ws "$KERNEL64_ELF" |
		awk -v sym="$symbol_name" '
            $8 == sym { print "0x"$2 }
        ')

	if [[ -z "$value" ]]; then
		printf "Error: symbol '%s' not found in %s\n" "$symbol_name" "$KERNEL64_ELF" >&2
		exit 1
	fi

	printf "%s\n" "$value"

}

__kernel_virtual_base=$(get_symbol "__kernel_virtual_base")
__module_end=$(get_symbol "__module_end")
__module_size=$(get_symbol "__module_size")

__text_start=$(get_symbol "__text_start")
__text_end=$(get_symbol "__text_end")

__rodata_start=$(get_symbol "__rodata_start")
__rodata_end=$(get_symbol "__rodata_end")

__data_start=$(get_symbol "__data_start")
__data_end=$(get_symbol "__data_end")

__bss_start=$(get_symbol "__bss_start")
__bss_end=$(get_symbol "__bss_end")

__stack_bottom=$(get_symbol "__stack_bottom")
__stack_top=$(get_symbol "__stack_top")

__heap_start=$(get_symbol "__heap_start")
__heap_end=$(get_symbol "__heap_end")

# Guard pages (add these extractions)
__text_guard_start=$(get_symbol "__text_guard_start")
__text_guard_end=$(get_symbol "__text_guard_end")

__rodata_guard_start=$(get_symbol "__rodata_guard_start")
__rodata_guard_end=$(get_symbol "__rodata_guard_end")

__data_guard_start=$(get_symbol "__data_guard_start")
__data_guard_end=$(get_symbol "__data_guard_end")

__bss_guard_start=$(get_symbol "__bss_guard_start")
__bss_guard_end=$(get_symbol "__bss_guard_end")

__stack_guard_start=$(get_symbol "__stack_guard_start")
__stack_guard_end=$(get_symbol "__stack_guard_end")

__heap_guard_start=$(get_symbol "__heap_guard_start")
__heap_guard_end=$(get_symbol "__heap_guard_end")

printf -- "\n\nModule size = %s" "$__module_size"

cat >"$OUTPUT_HEADER_FILE" <<EOF
/* =========================================================================
 *  AUTO-GENERATED FILE — DO NOT EDIT
 *
 *  Source ELF : $(basename "$KERNEL64_ELF")
 *  Symbol     : __module_size
 * =========================================================================
 */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KERNEL64_MODULE_SIZE ${__module_size}
#define KERNEL64_PAGE_COUNT ((KERNEL64_MODULE_SIZE + 0xFFF) / 0x1000)

/* Base addresses */
#define KERNEL64_VIRTUAL_BASE ${__kernel_virtual_base}

/* Module boundaries */
#define KERNEL64_MODULE_END ${__module_end}

/* Text section */
#define KERNEL64_TEXT_START ${__text_start}
#define KERNEL64_TEXT_END ${__text_end}
#define KERNEL64_TEXT_SIZE (KERNEL64_TEXT_END - KERNEL64_TEXT_START)

/* Text guard page */
#define KERNEL64_TEXT_GUARD_START ${__text_guard_start}
#define KERNEL64_TEXT_GUARD_END ${__text_guard_end}
#define KERNEL64_TEXT_GUARD_SIZE (KERNEL64_TEXT_GUARD_END - KERNEL64_TEXT_GUARD_START)

/* Read-only data section */
#define KERNEL64_RODATA_START ${__rodata_start}
#define KERNEL64_RODATA_END ${__rodata_end}
#define KERNEL64_RODATA_SIZE (KERNEL64_RODATA_END - KERNEL64_RODATA_START)

/* Rodata guard page */
#define KERNEL64_RODATA_GUARD_START ${__rodata_guard_start}
#define KERNEL64_RODATA_GUARD_END ${__rodata_guard_end}
#define KERNEL64_RODATA_GUARD_SIZE (KERNEL64_RODATA_GUARD_END - KERNEL64_RODATA_GUARD_START)

/* Data section */
#define KERNEL64_DATA_START ${__data_start}
#define KERNEL64_DATA_END ${__data_end}
#define KERNEL64_DATA_SIZE (KERNEL64_DATA_END - KERNEL64_DATA_START)

/* Data guard page */
#define KERNEL64_DATA_GUARD_START ${__data_guard_start}
#define KERNEL64_DATA_GUARD_END ${__data_guard_end}
#define KERNEL64_DATA_GUARD_SIZE (KERNEL64_DATA_GUARD_END - KERNEL64_DATA_GUARD_START)

/* BSS section */
#define KERNEL64_BSS_START ${__bss_start}
#define KERNEL64_BSS_END ${__bss_end}
#define KERNEL64_BSS_SIZE (KERNEL64_BSS_END - KERNEL64_BSS_START)

/* BSS guard page */
#define KERNEL64_BSS_GUARD_START ${__bss_guard_start}
#define KERNEL64_BSS_GUARD_END ${__bss_guard_end}
#define KERNEL64_BSS_GUARD_SIZE (KERNEL64_BSS_GUARD_END - KERNEL64_BSS_GUARD_START)

/* Stack section */
#define KERNEL64_STACK_BOTTOM ${__stack_bottom}
#define KERNEL64_STACK_TOP ${__stack_top}
#define KERNEL64_STACK_SIZE (KERNEL64_STACK_TOP - KERNEL64_STACK_BOTTOM)

/* Stack guard page */
#define KERNEL64_STACK_GUARD_START ${__stack_guard_start}
#define KERNEL64_STACK_GUARD_END ${__stack_guard_end}
#define KERNEL64_STACK_GUARD_SIZE (KERNEL64_STACK_GUARD_END - KERNEL64_STACK_GUARD_START)

/* Heap section */
#define KERNEL64_HEAP_START ${__heap_start}
#define KERNEL64_HEAP_END ${__heap_end}
#define KERNEL64_HEAP_SIZE (KERNEL64_HEAP_END - KERNEL64_HEAP_START)

/* Heap guard page */
#define KERNEL64_HEAP_GUARD_START ${__heap_guard_start}
#define KERNEL64_HEAP_GUARD_END ${__heap_guard_end}
#define KERNEL64_HEAP_GUARD_SIZE (KERNEL64_HEAP_GUARD_END - KERNEL64_HEAP_GUARD_START)

/* Section order verification macros */
#define ASSERT_SECTION_ORDER(prev_end, next_start) \
    ((prev_end) == (next_start) ? 0 : 1)

/* Check if address is within guard page (for debugging) */
#define IS_IN_GUARD_PAGE(addr) \
    (((addr) >= KERNEL64_TEXT_GUARD_START && (addr) < KERNEL64_TEXT_GUARD_END) || \
     ((addr) >= KERNEL64_RODATA_GUARD_START && (addr) < KERNEL64_RODATA_GUARD_END) || \
     ((addr) >= KERNEL64_DATA_GUARD_START && (addr) < KERNEL64_DATA_GUARD_END) || \
     ((addr) >= KERNEL64_BSS_GUARD_START && (addr) < KERNEL64_BSS_GUARD_END) || \
     ((addr) >= KERNEL64_STACK_GUARD_START && (addr) < KERNEL64_STACK_GUARD_END) || \
     ((addr) >= KERNEL64_HEAP_GUARD_START && (addr) < KERNEL64_HEAP_GUARD_END))

/* Total guard pages size */
#define KERNEL64_TOTAL_GUARD_SIZE \
    (KERNEL64_TEXT_GUARD_SIZE + \
     KERNEL64_RODATA_GUARD_SIZE + \
     KERNEL64_DATA_GUARD_SIZE + \
     KERNEL64_BSS_GUARD_SIZE + \
     KERNEL64_STACK_GUARD_SIZE + \
     KERNEL64_HEAP_GUARD_SIZE)

/* Total usable memory (excluding guard pages) */
#define KERNEL64_TOTAL_USABLE_SIZE \
    (KERNEL64_TEXT_SIZE + \
     KERNEL64_RODATA_SIZE + \
     KERNEL64_DATA_SIZE + \
     KERNEL64_BSS_SIZE + \
     KERNEL64_STACK_SIZE + \
     KERNEL64_HEAP_SIZE)

/* Convenience macros for section checking */
#define IS_IN_TEXT_SECTION(addr) \
    ((addr) >= KERNEL64_TEXT_START && (addr) < KERNEL64_TEXT_END)

#define IS_IN_RODATA_SECTION(addr) \
    ((addr) >= KERNEL64_RODATA_START && (addr) < KERNEL64_RODATA_END)

#define IS_IN_DATA_SECTION(addr) \
    ((addr) >= KERNEL64_DATA_START && (addr) < KERNEL64_DATA_END)

#define IS_IN_BSS_SECTION(addr) \
    ((addr) >= KERNEL64_BSS_START && (addr) < KERNEL64_BSS_END)

#define IS_IN_STACK_SECTION(addr) \
    ((addr) >= KERNEL64_STACK_BOTTOM && (addr) < KERNEL64_STACK_TOP)

#define IS_IN_HEAP_SECTION(addr) \
    ((addr) >= KERNEL64_HEAP_START && (addr) < KERNEL64_HEAP_END)

/* Memory region types for page table setup */
typedef enum {
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
static inline kernel64_region_t kernel64_get_region_type(uintptr_t addr) {
    if (IS_IN_TEXT_SECTION(addr)) return KERNEL64_REGION_TEXT;
    if (IS_IN_RODATA_SECTION(addr)) return KERNEL64_REGION_RODATA;
    if (IS_IN_DATA_SECTION(addr)) return KERNEL64_REGION_DATA;
    if (IS_IN_BSS_SECTION(addr)) return KERNEL64_REGION_BSS;
    if (IS_IN_STACK_SECTION(addr)) return KERNEL64_REGION_STACK;
    if (IS_IN_HEAP_SECTION(addr)) return KERNEL64_REGION_HEAP;
    if (IS_IN_GUARD_PAGE(addr)) return KERNEL64_REGION_GUARD;
    return KERNEL64_REGION_INVALID;
}

EOF

clang-format -i "$OUTPUT_HEADER_FILE"
