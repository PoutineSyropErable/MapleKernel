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
__bss_start
__bss_end

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

#define KERNEL64_MODULE_SIZE ${__module_size}

EOF
