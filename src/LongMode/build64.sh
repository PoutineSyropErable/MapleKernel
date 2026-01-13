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
  debug|release|fast       Build mode. Defaults to release if omitted.
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

print_array() {
	printf -- "\n\n%s:\n" "$2"
	local -n arr_ref=$1 # Use nameref to pass array by name
	for elem in "${arr_ref[@]}"; do
		echo "$elem"
	done
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

ZIG_CC="zig cc -target x86_64-freestanding"
MCMODEL="kernel"

ZIG_FLAGS=(
	"-target" "x86_64-freestanding"
	"-fno-stack-protector" # No stack protection
	"-fno-stack-check"     # No stack checking
	"-mcmodel=$MCMODEL"
)

ZIG_C_FLAGS=(
	"-std=gnu23"
	"-ffreestanding"
	"-mcmodel=$MCMODEL"
	"-mno-red-zone"           # Critical f
	"-fno-sanitize=undefined" # Disable UBSan
)

ZIG_C_LD_FLAGS=(
	"-nostdlib"
	"-ffreestanding"
	"-Wl,-z,max-page-size=0x1000"
	"-Wl,--gc-sections"
	"-Wl,--no-eh-frame-hdr"
	"-Wl,--entry=kernel64_start"          # <-- Tell linker about entry point
	"-Wl,--image-base=0xFFFFFFFF80000000" # MATCH your linker script!
	"-flto"
)

ZIG_LIB_LD_FLAGS=(
	"-static"
)

# Being generous with the cppflag
NASM_FLAGS64=("-f" "elf64")

NO_FPU_FLAGS=("-mno-sse" "-mno-sse2" "-mno-sse3" "-mno-sse4" "-mno-avx")
# FPU should be initialized

DEBUG_OPT_LVL="-O0"
RELEASE_OPT_LVL="-O3"

if [[ "$DEBUG_OR_RELEASE" == "debug" ]]; then
	echo "Debug mode enabled"

	NASM_FLAGS64+=("-g" "-F" "dwarf" "-DDEBUG")

	ZIG_C_FLAGS+=("$DEBUG_OPT_LVL" "-g" "-gdwarf-4" "-DDEBUG")
	ZIG_FLAGS+=("-O" "Debug"
		"-fPIC"
	)
	ZIG_C_LD_FLAGS+=("-g"
		"-fPIC"
	)

else
	echo "In normal mode, $RELEASE_OPT_LVL optimisation"

	ZIG_C_FLAGS+=(
		"-fno-PIC"
		"-fno-PIE"
	)

	RELEASE_MODE="ReleaseSafe"
	if [[ "$DEBUG_OR_RELEASE" == "fast" ]]; then
		RELEASE_MODE="ReleaseFast"
	fi
	ZIG_FLAGS+=("-O" "$RELEASE_MODE"
		"-fno-PIC"
		"-fno-PIE"
	)
	ZIG_C_LD_FLAGS+=(
		"-Wl,-no-pie"
	)
fi

if [[ "$QEMU_OR_REAL_MACHINE" == "qemu" ]]; then
	ZIG_C_FLAGS+=("-DQEMU")
	NASM_FLAGS64+=("-DQEMU")
fi

KERNEL64="./kernel64"
LONG_MODE_PREP32="../ProtectedMode/LongModePrep/"

rm -f "$BUILD_DIR/*.o"
rm -f "$BUILD_DIR/*.a"
rm -f "$BUILD_DIR/*.elf"

# print_array NASM_FLAGS64
print_array ZIG_C_FLAGS "ZIG_C_FLAGS"
print_array ZIG_C_LD_FLAGS "ZIG_C_LD_FLAGS"
print_array ZIG_FLAGS "ZIG_FLAGS"

printf -- "\n\n====== Assembly the Boot/Entry asm (And the guard pages) ========\n\n"
nasm "${NASM_FLAGS64[@]}" "$KERNEL64/kernel64_boot.asm" -o "$BUILD_DIR/kernel64_boot.o"
nasm "${NASM_FLAGS64[@]}" "$KERNEL64/guards.asm" -o "$BUILD_DIR/guard_pages.o"

printf -- "\n\n====== Compiling the Entry C code ========\n\n"
$ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/kernel_64.c" -o "$BUILD_DIR/kernel_64.o" "-I$LONG_MODE_PREP32"
$ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/com1.c" -o "$BUILD_DIR/com1.o"
$ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/dummy_kernel.c" -o "$BUILD_DIR/dummy_kernel.o"

# $ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/kernel64.zig" -o "$BUILD_DIR/kernel64_zig.o"
# $ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/std_options.zig" -o "$BUILD_DIR/std_options.o"
# $ZIG_CC "${ZIG_C_FLAGS[@]}" -c "$KERNEL64/file2.zig" -o "$BUILD_DIR/file2.o"

printf -- "\n\n====== Compiling the Zig library ========\n\n"
ZIG_LIB_NAME="kernel64"
zig build-lib "$ZIG_LIB_NAME.zig" "${ZIG_LIB_LD_FLAGS[@]}" "${ZIG_FLAGS[@]}" -femit-bin="$BUILD_DIR/lib$ZIG_LIB_NAME.a"

# zig build

# This single line can do a lot of work. Since it will build the whole thing

printf -- "\n\n====== Getting the '.o's and '.a's ========\n\n"
# Library configuration
LIBRARY_PATHS=(
	"$BUILD_DIR"
	"zig-out/lib"
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
$ZIG_CC \
	-T "linker_64.ld" \
	-o "$BUILD_DIR/kernel64.elf" \
	"${ZIG_C_LD_FLAGS[@]}" \
	"${BUILD_OBJECTS[@]}" \
	"${LIBRARY_ARGS[@]}" \
	-v

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
