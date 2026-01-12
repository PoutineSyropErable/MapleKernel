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

# Use zig cc for everything - it's Clang-based and perfectly compatible with Zig

GCC_FLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra" "-mcmodel=large")
GPP_FLAGS=("-std=gnu++23" "-ffreestanding" "-Wall" "-Wextra" "-fno-threadsafe-statics" "-fno-rtti" "-fno-exceptions" "-fno-strict-aliasing")

CLANG=clang

CLANG_FLAGS=(
	"-target" "x86_64-elf"
	"-std=gnu23"
	"-ffreestanding"
	"-nostdlib"
	"-fno-stack-protector"
	"-fno-stack-check"
	"-mcmodel=large"
	"-mno-red-zone" # Critical f
)

ZIG_CXX="zig c++ -target x86_64-freestanding"
ZIG_CC="zig cc -target x86_64-freestanding"
ZIG_C_FLAGS=(
	"-std=gnu23"
	"-ffreestanding"
	"-mcmodel=large"
	"-mno-red-zone"           # Critical f
	"-fno-sanitize=undefined" # Disable UBSan
)

ZIG_C_LD_FLAGS=(
	"-nostdlib"
	"-ffreestanding"
	"-no-pie" # Kernel shouldn't be PIE
	"-Wl,-z,max-page-size=0x1000"
	"-Wl,--gc-sections"
	"-Wl,--no-eh-frame-hdr"
	"-no-pie"
	"-Wl,--entry=kernel64_start" # <-- Tell linker about entry point
)

ZIG_FLAGS=(
	"-target" "x86_64-freestanding"
	"-fno-stack-protector" # No stack protection
	"-fno-stack-check"     # No stack checking
	"-mcmodel=large"
	"-fPIC"
)

ZIG_LIB_LD_FLAGS=("-static")

# Being generous with the cppflag
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc" "-fno-eliminate-unused-debug-symbols")
NASM_FLAGS64=("-f" "elf64")

NO_FPU_FLAGS=("-mno-sse" "-mno-sse2" "-mno-sse3" "-mno-sse4" "-mno-avx")
# Correct way to concatenate arrays in bash
GCC_FLAGS+=("${NO_FPU_FLAGS[@]}")
GPP_FLAGS+=("${NO_FPU_FLAGS[@]}")

DEBUG_OPT_LVL="-O0"
RELEASE_OPT_LVL="-O3"
#					  I RUWP
# a = 10 = 8 + 2 = 0000 1010
# -O1 in Cpp breaks printf option number and i have no idea why
QEMU_DBG_FLAGS=()

if [[ "$DEBUG_OR_RELEASE" == "debug" ]]; then
	echo "Debug mode enabled"
	GCC_FLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
	# GPPFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
	CFLAGS16+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	# LDFLAGS+=("-g")
	# NASM_FLAGS64+=("-g" "-F" "dwarf" "-DDEBUG")

	ZIG_C_FLAGS+=("$DEBUG_OPT_LVL" "-g" "-gdwarf-4" "-DDEBUG")
	ZIG_FLAGS+=("-O" "Debug")
	ZIG_LD_FLAGS+=("-g")

else
	echo "In normal mode, $RELEASE_OPT_LVL optimisation"
	GCC_FLAGS+=("$RELEASE_OPT_LVL")
	GPP_FLAGS+=("$RELEASE_OPT_LVL")
	LDFLAGS+=("$RELEASE_OPT_LVL")

	ZIG_FLAGS+=("-O" "ReleaseSafe")
fi

if [[ "$QEMU_OR_REAL_MACHINE" == "qemu" ]]; then
	GCC_FLAGS+=("-DQEMU")
fi

KERNEL64="./kernel64"

LONG_MODE_PREP32="../ProtectedMode/LongModePrep/"

rm -f "$BUILD_DIR/*.o"

printf -- "\n\n====== Assembly the Boot/Entry asm (And the guard pages) ========\n\n"
nasm "${NASM_FLAGS64[@]}" "$KERNEL64/kernel64_boot.asm" -o "$BUILD_DIR/kernel64_boot.o"
nasm "${NASM_FLAGS64[@]}" "$KERNEL64/guards.asm" -o "$BUILD_DIR/guard_pages.o"

printf -- "\n\n====== Compiling the Entry C code ========\n\n"
$ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/kernel_64.c" -o "$BUILD_DIR/kernel_64.o" "-I$LONG_MODE_PREP32"
$ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/com1.c" -o "$BUILD_DIR/com1.o"
$ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/dummy_kernel.c" -o "$BUILD_DIR/dummy_kernel.o"

printf -- "\n\n====== Compiling the Zig library ========\n\n"
ZIG_LIB_NAME="kernel64"
zig build-lib "$KERNEL64/$ZIG_LIB_NAME.zig" "${ZIG_LIB_LD_FLAGS[@]}" "${ZIG_FLAGS[@]}" -femit-bin="$BUILD_DIR/lib$ZIG_LIB_NAME.a"
# This single line can do a lot of work. Since it will build the whole thing

printf -- "\n\n====== Getting the '.o's and '.a's ========\n\n"
# Library configuration
LIBRARY_PATHS=(
	"$BUILD_DIR"
	# "/path/to/other/libs"
)

LIBRARY_FILES=(
	"$ZIG_LIB_NAME" # don't put the lib or the
	# "libother.a"
)

# Build the -L and library arguments
LIBRARY_ARGS=()
for path in "${LIBRARY_PATHS[@]}"; do
	LIBRARY_ARGS+=("-L$path")
done

for lib in "${LIBRARY_FILES[@]}"; do
	LIBRARY_ARGS+=("-l$lib")
done

BUILD_OBJECTS=("$BUILD_DIR"/*.o)

printf -- "\n\n====================== Linking ===============================\n\n"
# $GCC64 -T "linker_64.ld" -o "$BUILD_DIR/kernel64.elf" "${LDFLAGS[@]}" "${BUILD_OBJECTS[@]}" "${LIBRARY_ARGS[@]}"

$ZIG_CC \
	-T "linker_64.ld" \
	-o "$BUILD_DIR/kernel64.elf" \
	"${ZIG_C_LD_FLAGS[@]}" \
	"${BUILD_OBJECTS[@]}" \
	"${LIBRARY_ARGS[@]}" \
	-v
# Use LLVM's linker (better with Zig DWARF)

# Use zig build-exe or zig ld to link everything
# zig build-exe \
# 	"${BUILD_OBJECTS[@]}" \
# 	"$KERNEL64/kernel64.zig" \
# 	"${ZIGFLAGS[@]}" \
# 	-T "linker_64.ld" \
# 	-femit-bin="$BUILD_DIR/kernel64.elf" \
# 	--verbose-link

objdump -D -h -M intel "$BUILD_DIR/kernel64.elf" >"$BUILD_DIR/kernel64.dump"

printf -- "\n\n====== Copying ========\n\n"
cp "$BUILD_DIR/kernel64.elf" "$ISO_DIR/boot/kernel64.elf"

#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
