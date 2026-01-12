#!/usr/bin/env bash

set -eou pipefail

BUILD32_DIR="../build32"
BUILD64_DIR="../build64"

LONG_MODE_PREP="./ProtectedMode/LongModePrep/"
OUTPUT_HEADER_FILE="$LONG_MODE_PREP/kernel64_size.hpp"
OUTPUT_INCLUDE_FILE="$LONG_MODE_PREP/kernel64_size.inc"

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
		# printf "Error: symbol '%s' not found in %s\n" "$symbol_name" "$KERNEL64_ELF" >&2
		# exit 1
		printf "0\n"
	fi

	printf "%s\n" "$value"

}

__kernel_virtual_base=$(get_symbol "__kernel_virtual_base")
__module_end=$(get_symbol "__module_end")
__module_size=$(get_symbol "__module_size")
kernel64_start=$(get_symbol "kernel64_start")
kernel64_main=$(get_symbol "kernel64_main")

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

printf -- "\n\n\nModule size = %s\n" "$__module_size"
printf -- "Kernel 64 main (address) = %s" "$kernel64_main"

cat >"$OUTPUT_INCLUDE_FILE" <<EOF
; =========================================================================
;  AUTO-GENERATED FILE — DO NOT EDIT
;
;  Source ELF : $(basename "$KERNEL64_ELF")
; =========================================================================
;
    VIRTUAL_BASE equ ${__kernel_virtual_base};
    KERNEL64_ENTRY equ ${kernel64_start};
	KERNEL64_ENTRY_PHYSICAL equ 0x10a000

    KERNEL64_MAIN_ADDR equ ${kernel64_main};
EOF

cat >"$OUTPUT_HEADER_FILE" <<EOF
/* =========================================================================
 *  AUTO-GENERATED FILE — DO NOT EDIT
 *
 *  Source ELF : $(basename "$KERNEL64_ELF")
 * =========================================================================
 */

#pragma once
#include <stddef.h>
#include <stdint.h>
#include "static_assert.h"

namespace kernel64_size {

    /* Module size & page count */
    constexpr uint64_t MODULE_SIZE = ${__module_size};
    constexpr uint64_t PAGE_COUNT = (MODULE_SIZE + 0xFFF) / 0x1000;
	STATIC_ASSERT(PAGE_COUNT < 0xFFFF'FFFF, "Page count must fit in a uint32_t number\n");

    /* Base addresses */
    constexpr uint64_t VIRTUAL_BASE = ${__kernel_virtual_base};
    constexpr uint64_t KERNEL64_MAIN_ADDR = ${kernel64_main};

    /* Module end */
    constexpr uint64_t MODULE_END = ${__module_end};

    /* Text section */
    constexpr uint64_t TEXT_START = ${__text_start};
    constexpr uint64_t TEXT_END   = ${__text_end};
    constexpr uint64_t TEXT_SIZE  = TEXT_END - TEXT_START;

    /* Text guard page */
    constexpr uint64_t TEXT_GUARD_START = ${__text_guard_start};
    constexpr uint64_t TEXT_GUARD_END   = ${__text_guard_end};
    constexpr uint64_t TEXT_GUARD_SIZE  = TEXT_GUARD_END - TEXT_GUARD_START;

    /* Read-only data section */
    constexpr uint64_t RODATA_START = ${__rodata_start};
    constexpr uint64_t RODATA_END   = ${__rodata_end};
    constexpr uint64_t RODATA_SIZE  = RODATA_END - RODATA_START;

    /* Rodata guard page */
    constexpr uint64_t RODATA_GUARD_START = ${__rodata_guard_start};
    constexpr uint64_t RODATA_GUARD_END   = ${__rodata_guard_end};
    constexpr uint64_t RODATA_GUARD_SIZE  = RODATA_GUARD_END - RODATA_GUARD_START;

    /* Data section */
    constexpr uint64_t DATA_START = ${__data_start};
    constexpr uint64_t DATA_END   = ${__data_end};
    constexpr uint64_t DATA_SIZE  = DATA_END - DATA_START;

    /* Data guard page */
    constexpr uint64_t DATA_GUARD_START = ${__data_guard_start};
    constexpr uint64_t DATA_GUARD_END   = ${__data_guard_end};
    constexpr uint64_t DATA_GUARD_SIZE  = DATA_GUARD_END - DATA_GUARD_START;

    /* BSS section */
    constexpr uint64_t BSS_START = ${__bss_start};
    constexpr uint64_t BSS_END   = ${__bss_end};
    constexpr uint64_t BSS_SIZE  = BSS_END - BSS_START;

    /* BSS guard page */
    constexpr uint64_t BSS_GUARD_START = ${__bss_guard_start};
    constexpr uint64_t BSS_GUARD_END   = ${__bss_guard_end};
    constexpr uint64_t BSS_GUARD_SIZE  = BSS_GUARD_END - BSS_GUARD_START;

    /* Stack section */
    constexpr uint64_t STACK_BOTTOM = ${__stack_bottom};
    constexpr uint64_t STACK_TOP    = ${__stack_top};
    constexpr uint64_t STACK_SIZE   = STACK_TOP - STACK_BOTTOM;

    /* Stack guard page */
    constexpr uint64_t STACK_GUARD_START = ${__stack_guard_start};
    constexpr uint64_t STACK_GUARD_END   = ${__stack_guard_end};
    constexpr uint64_t STACK_GUARD_SIZE  = STACK_GUARD_END - STACK_GUARD_START;

    /* Heap section */
    constexpr uint64_t HEAP_START = ${__heap_start};
    constexpr uint64_t HEAP_END   = ${__heap_end};
    constexpr uint64_t HEAP_SIZE  = HEAP_END - HEAP_START;

    /* Heap guard page */
    constexpr uint64_t HEAP_GUARD_START = ${__heap_guard_start};
    constexpr uint64_t HEAP_GUARD_END   = ${__heap_guard_end};
    constexpr uint64_t HEAP_GUARD_SIZE  = HEAP_GUARD_END - HEAP_GUARD_START;

    /* Total guard pages */
    constexpr uint64_t TOTAL_GUARD_SIZE =
        TEXT_GUARD_SIZE + RODATA_GUARD_SIZE + DATA_GUARD_SIZE +
        BSS_GUARD_SIZE + STACK_GUARD_SIZE + HEAP_GUARD_SIZE;

    /* Total usable memory */
    constexpr uint64_t TOTAL_USABLE_SIZE =
        TEXT_SIZE + RODATA_SIZE + DATA_SIZE + BSS_SIZE + STACK_SIZE + HEAP_SIZE;

    /* Address range helpers */
    constexpr bool is_in_text(uint64_t addr)  { return addr >= TEXT_START && addr < TEXT_END; }
    constexpr bool is_in_rodata(uint64_t addr){ return addr >= RODATA_START && addr < RODATA_END; }
    constexpr bool is_in_data(uint64_t addr)  { return addr >= DATA_START && addr < DATA_END; }
    constexpr bool is_in_bss(uint64_t addr)   { return addr >= BSS_START && addr < BSS_END; }
    constexpr bool is_in_stack(uint64_t addr) { return addr >= STACK_BOTTOM && addr < STACK_TOP; }
    constexpr bool is_in_heap(uint64_t addr)  { return addr >= HEAP_START && addr < HEAP_END; }
    constexpr bool is_in_guard(uint64_t addr) {
        return (addr >= TEXT_GUARD_START && addr < TEXT_GUARD_END) ||
               (addr >= RODATA_GUARD_START && addr < RODATA_GUARD_END) ||
               (addr >= DATA_GUARD_START && addr < DATA_GUARD_END) ||
               (addr >= BSS_GUARD_START && addr < BSS_GUARD_END) ||
               (addr >= STACK_GUARD_START && addr < STACK_GUARD_END) ||
               (addr >= HEAP_GUARD_START && addr < HEAP_GUARD_END);
    }

    /* Section order verification */
    constexpr bool assert_section_order(uint64_t prev_end, uint64_t next_start) {
        return prev_end == next_start;
    }

    /* Memory region types */
    enum class region_t : uint8_t {
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
    constexpr region_t get_region_type(uint64_t addr) {
        return is_in_text(addr)   ? region_t::TEXT   :
               is_in_rodata(addr) ? region_t::RODATA :
               is_in_data(addr)   ? region_t::DATA   :
               is_in_bss(addr)    ? region_t::BSS    :
               is_in_stack(addr)  ? region_t::STACK  :
               is_in_heap(addr)   ? region_t::HEAP   :
               is_in_guard(addr)  ? region_t::GUARD  :
                                    region_t::INVALID;
    }


	constexpr const char* region_to_string(region_t r) {
		switch (r) {
			case region_t::TEXT:   return "TEXT";
			case region_t::RODATA: return "RODATA";
			case region_t::DATA:   return "DATA";
			case region_t::BSS:    return "BSS";
			case region_t::STACK:  return "STACK";
			case region_t::HEAP:   return "HEAP";
			case region_t::GUARD:  return "GUARD";
			case region_t::INVALID:return "INVALID";
		}
		return "UNKNOWN"; // fallback (unreachable if all enum values covered)
	}

} // namespace kernel64_size

EOF

clang-format -i "$OUTPUT_HEADER_FILE"
