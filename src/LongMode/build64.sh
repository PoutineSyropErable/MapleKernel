#!/usr/bin/env bash

set -eou pipefail
shopt -s nullglob

# If you have qemu full
QEMU_FULL=true
# CONTROL + ALT + G to get the mouse back (GTK), default on my machine
# or CTRL+ALT (SDL)

DEBUG_OR_RELEASE="${1:-release}"
QEMU_OR_REAL_MACHINE="${2:-qemu}"

# Check for help argument
if [[ "${1:-}" == "help" ]]; then
	cat <<'EOF'
Usage: ./build.sh [debug|release] [QEMU|REAL] [32|64] [move|nomove]

Arguments:
  debug|release       Build mode. Defaults to release if omitted.
  qemu|real           Whether to run in QEMU or on a real machine. Defaults to QEMU.

Examples:
  ./build.sh debug QEMU
  ./build.sh release REAL

Notes:
  - debug enables -g and -DDEBUG flags
  - QEMU_OR_REAL_MACHINE chooses whether to start QEMU, and add -QEMU flags

EOF
	exit 0
fi

# use my custom gcc and g++:
if true; then
	PATH="$HOME/cross-gcc/install-ia16-elf/bin:$PATH"
	PATH="$HOME/cross-gcc/install-i686-elf/bin:$PATH"
	PATH="$HOME/cross-gcc/install-x86_64-elf/bin:$PATH"
	export PATH
fi

function find_git_root() {
	local dir=${1:-$PWD} # start from given dir or current directory
	while [[ "$dir" != "/" ]]; do
		if [[ -d "$dir/.git" ]]; then
			echo "$dir"
			return 0
		fi
		dir=$(dirname "$dir")
	done
	echo "No git repository found" >&2
	return 1
}

project_root=$(find_git_root)
src="$project_root/src/LongMode"
cd "$src" || { # Tries to cd to "src" relative to current dir
	echo "Could not cd to $src."
	exit 1
}

BUILD_DIR="../../build64"
ISO_DIR="../../isodir"

mkdir -p "$BUILD_DIR"

GCC64=x86_64-elf-gcc
GPP64=x86_64-elf-g++

CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra" "-mcmodel=kernel")
CPPFLAGS=("-std=gnu++23" "-ffreestanding" "-Wall" "-Wextra" "-fno-threadsafe-statics" "-fno-rtti" "-fno-exceptions" "-fno-strict-aliasing")
# Being generous with the cppflag
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc" "-fno-eliminate-unused-debug-symbols")
NASM_FLAGS64=("-f" "elf64")

DEBUG_OPT_LVL="-O3"
RELEASE_OPT_LVL="-O0"
#					  I RUWP
# a = 10 = 8 + 2 = 0000 1010
# -O1 in Cpp breaks printf option number and i have no idea why
QEMU_DBG_FLAGS=()

if [[ "$DEBUG_OR_RELEASE" == "debug" ]]; then
	echo "Debug mode enabled"
	CFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
	CPPFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
	CFLAGS16+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	LDFLAGS+=("-g")
	NASM_FLAGS64+=("-g" "-F" "dwarf" "-DDEBUG")
else
	echo "In normal mode, $RELEASE_OPT_LVL optimisation"
	CFLAGS+=("$RELEASE_OPT_LVL")
	CPPFLAGS+=("$RELEASE_OPT_LVL")
	LDFLAGS+=("$RELEASE_OPT_LVL")
fi

if [[ "$QEMU_OR_REAL_MACHINE" == "qemu" ]]; then
	CFLAGS+=("-DQEMU")
fi

KERNEL64="./kernel64"

rm -f "$BUILD_DIR/*.o"

$GCC64 "${CFLAGS[@]}" -c "$KERNEL64/kernel_64.c" -o "$BUILD_DIR/kernel_64.o"
$GCC64 "${CFLAGS[@]}" -c "$KERNEL64/com1.c" -o "$BUILD_DIR/com1.o"
$GCC64 "${CFLAGS[@]}" -c "$KERNEL64/dummy_kernel.c" -o "$BUILD_DIR/dummy_kernel.o"
nasm "${NASM_FLAGS64[@]}" "$KERNEL64/kernel64_boot.asm" -o "$BUILD_DIR/kernel64_boot.o"

BUILD_OBJECTS=("$BUILD_DIR"/*.o)
printf -- "\n\n====== Linking ========\n\n"
$GPP64 -T "linker_64.ld" -o "$BUILD_DIR/kernel64.elf" "${LDFLAGS[@]}" "${BUILD_OBJECTS[@]}" "${LIBRARY_ARGS[@]}"

objdump -D -h -M intel "$BUILD_DIR/kernel64.elf" >"$BUILD_DIR/kernel64.dump"

printf -- "\n\n====== Copying ========\n\n"
cp "$BUILD_DIR/kernel64.elf" "$ISO_DIR/boot/kernel64.elf"
