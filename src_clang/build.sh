#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
ISO_DIR="isodir"
KERNEL_DIR="kernel"
STDIO_DIR="./stdio"

CLANG_INSERTS_DIR="./clang_helpers"
MODULES_DIR="./modules"
MODULE_CACHE_DIR="$BUILD_DIR/module_cache"

mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub" "$MODULE_CACHE_DIR"

# ============================================================================
# GCC FLAGS (for linking)
# ============================================================================
CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra")
NASM_FLAGS32=("-f" "elf32")
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc" "-fno-eliminate-unused-debug-symbols")

# ============================================================================
# CLANG FLAGS (for compilation)
# ============================================================================
# Common flags for both C and C++
COMMON_CLANG_FLAGS=(
	"-target" "i686-elf"
	"-ffreestanding"
	"-Wall"
	"-Wextra"
	"-fno-builtin"
	"-nostdlib"
	"-nostdlibinc"
	"-O1"
	"-mno-sse"
	"-mno-mmx"
	"-m32"
	"-fno-stack-protector"
	"-fno-single-precision-constant"
	"-I$STDIO_DIR"
	"-I$CLANG_INSERTS_DIR"
	"-I$MODULES_DIR"
)

# C-specific flags
C_CLANG_FLAGS=(
	"-std=gnu23"
	"-fno-unwind-tables"
)

# C++20 modules flags - FIXED: removed -fmodules-debuginfo, added -Xclang
CPP_CLANG_FLAGS=(
	"-std=gnu++20" # Use gnu++20 for better GCC compatibility
	"-fmodules"    # Enable modules
	"-fmodules-cache-path=$MODULE_CACHE_DIR"
	"-Xclang" "-fmodules-local-submodule-visibility"
	"-Xclang" "-fmodules-debuginfo" # Fixed: needs -Xclang prefix
	"-fno-exceptions"
	"-fno-rtti"
	"-fno-threadsafe-statics"
	"-fno-use-cxa-atexit"
	"-fno-unwind-tables"
)

# Kernel extra flags
KERNEL_EXTRA_FLAGS=(
	"-mno-red-zone"
	"-mgeneral-regs-only"
	"-fno-pic"
	"-fno-pie"
	"-fno-omit-frame-pointer"
)

# Combined flag sets
CLANG_FLAGS=("${COMMON_CLANG_FLAGS[@]}" "${C_CLANG_FLAGS[@]}")
CLANGPP_FLAGS=("${COMMON_CLANG_FLAGS[@]}" "${CPP_CLANG_FLAGS[@]}")
CLANG_KERNEL_FLAGS=("${CLANG_FLAGS[@]}" "${KERNEL_EXTRA_FLAGS[@]}")
CLANGPP_KERNEL_FLAGS=("${CLANGPP_FLAGS[@]}" "${KERNEL_EXTRA_FLAGS[@]}")

# ============================================================================
# MODULE FORMAT SELECTION
# ============================================================================
# Set this to either "ixx" for single-file modules or "cppm" for two-file modules
MODULE_FORMAT="cppm" # Change to "cppm" if using two-file modules

# ============================================================================
# COMPILATION (using Clang)
# ============================================================================

echo "Building bootloader..."
nasm "${NASM_FLAGS32[@]}" "boot_intel.asm" -o "$BUILD_DIR/boot.o"

echo "Building C kernel..."
clang "${CLANG_KERNEL_FLAGS[@]}" -c "kernel.c" -o "$BUILD_DIR/kernel.o" "-I./kernel"

echo "Building Stdio..."
clang "${CLANG_KERNEL_FLAGS[@]}" \
	-c "$STDIO_DIR/stdio.c" \
	-o "$BUILD_DIR/stdio.o"

clang "${CLANG_KERNEL_FLAGS[@]}" \
	-c "$CLANG_INSERTS_DIR/float_helpers.c" \
	-o "$BUILD_DIR/float_helpers.o"

./modules/build.sh

# ============================================================================
# LINKING (using GCC/g++ - keeping your original approach)
# ============================================================================
echo "Linking kernel..."

# Collect all object files for linking - FIXED: missing $ for array declaration
LINK_OBJECTS=(
	"$BUILD_DIR/boot.o"
	"$BUILD_DIR/kernel.o"
	"$BUILD_DIR/stdio.o"
	"$BUILD_DIR/float_helpers.o"
	"$BUILD_DIR/A.o"
	"$BUILD_DIR/B.o"
	"$BUILD_DIR/C.o"
	"$BUILD_DIR/D.o"
	"$BUILD_DIR/C_impl.o"
	"$BUILD_DIR/D_impl.o"
	"$BUILD_DIR/Main.o"
	"$BUILD_DIR/Main_cpp.o"

)

# Add module object files based on format
if [ "$MODULE_FORMAT" = "ixx" ] && [ -f "$BUILD_DIR/module.o" ]; then
	LINK_OBJECTS+=("$BUILD_DIR/module.o")
elif [ "$MODULE_FORMAT" = "cppm" ]; then
	if [ -f "$BUILD_DIR/module_impl.o" ]; then
		LINK_OBJECTS+=("$BUILD_DIR/module_impl.o")
	fi
fi

echo "Linking with C++ support (g++)..."
i686-elf-g++ -T linker.ld -o "$BUILD_DIR/myos.elf" \
	"${LDFLAGS[@]}" \
	"${LINK_OBJECTS[@]}"
# ============================================================================
# MULTIBOOT VERIFICATION
# ============================================================================
USE_MULTIBOOT1=false
if [ "$USE_MULTIBOOT1" == true ]; then
	if grub-file --is-x86-multiboot "$BUILD_DIR/myos.elf"; then
		echo "Multiboot confirmed"
	else
		echo "The file is not multiboot"
		exit 1
	fi
else
	if grub-file --is-x86-multiboot2 "$BUILD_DIR/myos.elf"; then
		echo "Multiboot2 confirmed"
	else
		echo "The file is not multiboot 2"
		exit 1
	fi
fi

# ============================================================================
# CREATE ISO
# ============================================================================
cp "$BUILD_DIR/myos.elf" "$ISO_DIR/boot/myos.elf"
if [ -f "$ISO_DIR/grub.cfg" ]; then
	cp "$ISO_DIR/grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"
elif [ -f "grub.cfg" ]; then
	cp "grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"
else
	echo "Warning: No grub.cfg found, creating default..."
	cat >"$ISO_DIR/boot/grub/grub.cfg" <<'EOF'
menuentry "MyOS" {
    multiboot2 /boot/myos.elf
    boot
}
EOF
fi

grub-mkrescue -o "$BUILD_DIR/myos.iso" "$ISO_DIR"
echo "ISO created successfully: $BUILD_DIR/myos.iso"

# ============================================================================
# RUN QEMU
# ============================================================================
QEMU32=qemu-system-i386
QEMU64=qemu-system-x86_64
QEMU="$QEMU64"

# Check if QEMU is already running, kill only if it exists
if pgrep -f "$QEMU" >/dev/null 2>&1; then
	echo "Killing existing QEMU process..."
	pkill -f "$QEMU" 2>/dev/null
	sleep 1 # Give it time to die
fi

$QEMU \
	-cdrom "$BUILD_DIR/myos.iso" \
	-no-reboot \
	-serial stdio &

QEMU_PID=$!

sleep 1

# Launch VNC viewer
if command -v vncviewer >/dev/null 2>&1; then
	vncviewer localhost:5900 &
	VNC_PID=$!

	# sleep 1
	if [[ "${MOVE_VNC:-}" == "move" ]]; then
		move_pid_to_workspace $VNC_PID 21 2>/dev/null || true
	fi

	wait $VNC_PID 2>/dev/null || true
else
	echo "vncviewer not found, continuing without VNC..."
	wait $QEMU_PID
fi

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null || true
