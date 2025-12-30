#!/usr/bin/env bash

set -eou pipefail
shopt -s nullglob

DEBUG_OR_RELEASE="${1:-release}"
QEMU_OR_REAL_MACHINE="${2:-qemu}"

# Check for help argument
if [[ "${1:-}" == "help" ]]; then
	cat <<'EOF'
Usage: ./build.sh [debug|release] [QEMU|REAL] [32|64] [move|nomove]

Arguments:
  debug|release       Build mode. Defaults to release if omitted.
  qemu|real           Whether to run in QEMU or on a real machine. Defaults to QEMU.
  32|64               Machine bitness. Defaults to 64.
  move|nomove         Whether to move VNC window to workspace. Defaults to move.

Examples:
  ./build.sh debug QEMU 64 move
  ./build.sh release REAL 32 nomove

Notes:
  - debug enables -g and -DDEBUG flags
  - QEMU_OR_REAL_MACHINE chooses whether to start QEMU, and add -QEMU flags
  - MACHINE_BITNESS chooses QEMU binary (qemu-system-i386 vs qemu-system-x86_64)
  - MOVE_VNC controls whether VNC viewer is moved to a workspace

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
src="$project_root/src/ProtectedMode"
cd "$src" || { # Tries to cd to "src" relative to current dir
	echo "Could not cd to $src."
	exit 1
}

#With this, it should work nice
export PATH="../user_tools:$PATH"
source ../user_tools/source_this.bash

# Define directories
BUILD_DIR="../../build32"
ISO_DIR="../../isodir"

GCC32=i686-elf-gcc
GPP32=i686-elf-g++
GCC64=x86_64-elf-gcc
GPP64=x86_64-elf-g++

CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra")
CPPFLAGS=("-std=gnu++23" "-ffreestanding" "-Wall" "-Wextra" "-fno-threadsafe-statics" "-fno-rtti" "-fno-exceptions" "-fno-strict-aliasing")
# Being generous with the cppflag
CFLAGS16=("-std=gnu99" "-ffreestanding" "-Wall" "-Wextra")
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc" "-fno-eliminate-unused-debug-symbols")
NASM_FLAGS32=("-f" "elf32")
NASM_FLAGS16=("-f" "elf")

DEBUG_OPT_LVL="-O3"
RELEASE_OPT_LVL="-O3"
# -O1 in Cpp breaks printf option number and i have no idea why
QEMU_DBG_FLAGS=()

if [[ "$DEBUG_OR_RELEASE" == "debug" ]]; then
	echo "Debug mode enabled"
	CFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
	CPPFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
	CFLAGS16+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	LDFLAGS+=("-g")
	NASM_FLAGS32+=("-g" "-F" "dwarf" "-DDEBUG")
	NASM_FLAGS16+=("-g" "-F" "dwarf" "-DDEBUG")
	QEMU_DBG_FLAGS+=("-s" "-S")
else
	echo "In normal mode, $RELEASE_OPT_LVL optimisation"
	CFLAGS+=("$RELEASE_OPT_LVL")
	CPPFLAGS+=("$RELEASE_OPT_LVL")
	CFLAGS16+=("$RELEASE_OPT_LVL")
	LDFLAGS+=("$RELEASE_OPT_LVL")
fi

if [[ "$QEMU_OR_REAL_MACHINE" == "qemu" ]]; then
	CFLAGS+=("-DQEMU")
fi

# QEMU-Compatible FPU flags for 32-bit
FPU32_FLAGS_COMMON=(
	"-m32"
	"-march=pentium4" # QEMU supports SSE2, SSE3
	"-mfpmath=sse"    # Use SSE registers instead of x87
	"-msse"           # Enable SSE
	"-msse2"          # Enable SSE2
	"-msse3"          # Enable SSE3
)

# GCC-specific FPU extras
FPU32_FLAGS_GCC=(
	"${FPU32_FLAGS_COMMON[@]}"
	"-mtune=generic" # GCC-specific tuning
	"-mno-avx"       # Disable AVX (QEMU doesn't have it)
	"-mno-avx2"      # Disable AVX2
	"-mno-avx512f"   # Disable AVX512
	"-mstackrealign" # Backup/extra safety. Stack alligned to 16 byte. So, some sse operation don't crash
	# But this, will and might cause issues in my embedded assembly code
)

# Clang-specific FPU extras
FPU32_FLAGS_CLANG=(
	"${FPU32_FLAGS_COMMON[@]}"
	"-mstackrealign" # Clang needs this for 32-bit
	"-mno-avx"       # Disable AVX
	"-mno-avx2"      # Disable AVX2
	"-mno-avx512f"   # Disable AVX512
)

CFLAGS_NOFPU=("${CFLAGS[@]}")
CPPFLAGS_NOFPU=("${CPPFLAGS[@]}")
CFLAGS+=("${FPU32_FLAGS_GCC[@]}")
CPPFLAGS+=("${FPU32_FLAGS_GCC[@]}")

# Create necessary directories
mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

# The variable for the dirs:
KERNEL="kernel"
KERNEL_CPP="kernel/cpp"
REAL16_WRAPPERS="real16_wrappers"
REAL_FUNC="realmode_functions"
STDIO="stdio"
STDLIB="stdlib"
OTHER="other"
CODE_ANALYSIS="runtime_code_analysis"

DRIVERS="./drivers"

PS2="./drivers/ps2"
PS2_CONTROLLER="./drivers/ps2/controller"
PS2_KEYBOARD="./drivers/ps2/keyboard"
PS2_MOUSE="./drivers/ps2/mouse"
PS2_KEYBOARD_CPP="./drivers/ps2/keyboard/cpp"

DRIVERS_USB_CONTROLLER="./drivers/usb/controller"

BIOS="./firmware/bios"
EFI="./firmware/efi"
UEFI="./firmware/uefi"
MULTIBOOT="./firmware/multiboot"
ACPI="./firmware/acpi"

CPU="./cpu/"
FPU="./cpu/fpu"
CONTROL_REGISTERS="./cpu/control_registers"
CPUID="./cpu/cpuid"
PIC="./cpu/pic"
APIC="./cpu/apic"
APIC_IO="./cpu/apic_io"
MULTICORE="./cpu/multicore"
GDT="./cpu/gdt"
IDT="./cpu/idt"

CPP="./z_otherLang/cpp/"
RUST="./z_otherLang/rust/"
ZIG="./z_otherLang/zig/"

FRAMEBUFER="./framebuffer"
TIMERS="./timers"
PIT="./timers/pit"

MODULES="./modules/"

INCLUDE_DIRS=(
	"$KERNEL"
	"$REAL16_WRAPPERS"
	"$REAL_FUNC"
	"$STDIO"
	"$STDLIB"
	"$OTHER"
	"$CODE_ANALYSIS"

	"$PS2"
	"$PS2_CONTROLLER"
	"$PS2_KEYBOARD"
	"$PS2_KEYBOARD_CPP"
	"$PS2_MOUSE"

	"$DRIVERS_USB_CONTROLLER"

	"$CPU"
	"$CPUID"
	"$FPU"
	"$CONTROL_REGISTERS"
	"$GDT"
	"$IDT"
	"$PIC"
	"$APIC"
	"$APIC_IO"
	"$MULTICORE"

	"$BIOS"
	"$EFI"
	"$UEFI"
	"$MULTIBOOT"
	"$ACPI"
	"$ACPI/madt"

	"$FRAMEBUFER"
	"$TIMERS"
	"$PIT"

	"$KERNEL_CPP"
	"$RUST"
	"$ZIG"
	"$MODULES"
)

PS2_INCLUDE_DIRS=(
	"$PS2"
	"$PS2_CONTROLLER"
	"$PS2_KEYBOARD"
	"$PS2_KEYBOARD_CPP"
	"$PS2_MOUSE"
	"$PIC"
	"$CPU"
)

# Build SUPER_INCLUDE as an array of -I arguments
SUPER_INCLUDE=()
for dir in "${INCLUDE_DIRS[@]}"; do
	SUPER_INCLUDE+=("-I$dir")
done

PS2_SUPER_INCLUDE=()
for dir in "${PS2_INCLUDE_DIRS[@]}"; do
	PS2_SUPER_INCLUDE+=("-I$dir")
done

# $GPP32 "${CPPFLAGS[@]}" -c "./find_all_includer.cpp" -o "$BUILD_DIR/find_all.o" "${SUPER_INCLUDE[@]}"

# Assemble the bootloader assembly
nasm "${NASM_FLAGS32[@]}" "$KERNEL/boot_intel.asm" -o "$BUILD_DIR/boot.o"
$GCC32 "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$KERNEL/kernel.c" -o "$BUILD_DIR/kernel.o"
$GCC32 "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$KERNEL/symbols.c" -o "$BUILD_DIR/symbols.o"
$GCC32 "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$KERNEL/kernel_helper.c" -o "$BUILD_DIR/kernel_helper.o"
$GPP32 "${CPPFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$MULTIBOOT/multiboot.cpp" -o "$BUILD_DIR/multiboot.o"
$GPP32 "${CPPFLAGS[@]}" -c "$KERNEL_CPP/kernel_cpp.cpp" -o "$BUILD_DIR/kernel_cpp.o" "${SUPER_INCLUDE[@]}"
$GPP32 "${CPPFLAGS[@]}" -c "$GDT/multicore_gdt.cpp" -o "$BUILD_DIR/multicore_gdt.o" "${SUPER_INCLUDE[@]}"
$GCC32 "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$MULTIBOOT/multiboot_module.c" -o "$BUILD_DIR/multiboot_modc.o"

# Firmware
$GCC32 "${CPPFLAGS[@]}" -c "$ACPI/acpi.c" -o "$BUILD_DIR/acpi_c.o" "-I$STDLIB" "-I$STDIO"
$GPP32 "${CPPFLAGS[@]}" -c "$ACPI/acpi.cpp" -o "$BUILD_DIR/acpi.o" "-I$STDLIB" "-I$STDIO" "-I$ACPI/madt" "-I$ACPI"
$GPP32 "${CPPFLAGS[@]}" -c "$ACPI/madt/madt.cpp" -o "$BUILD_DIR/madt.o" "-I$STDLIB" "-I$STDIO" "-I$ACPI" "-I$APIC" "-I$ACPI/madt"

# Compile the print functions.
$GCC32 "${CFLAGS[@]}" -c "$STDIO/string_helper.c" -o "$BUILD_DIR/string_helper.o"
$GCC32 "${CFLAGS[@]}" -c "$STDIO/bit_hex_string.c" -o "$BUILD_DIR/bit_hex_string.o" -std=gnu99 "-I$STDIO" "-I$STDLIB"
$GCC32 "${CFLAGS[@]}" -c "$STDIO/vga_terminal.c" -o "$BUILD_DIR/vga_terminal.o" "-I$STDIO" "-I$OTHER" "-I$STDLIB" "-I$PIT"
$GCC32 "${CFLAGS[@]}" -c "$STDIO/stdio.c" -o "$BUILD_DIR/stdio.o" "-I$STDIO" "-I$OTHER" "-I$STDLIB" "-I$MULTICORE"

#compile misc helper functions
$GCC32 "${CFLAGS[@]}" -c "$STDLIB/stdlib.c" -o "$BUILD_DIR/stdlib.o" "-I$STDLIB" "-I$STDIO"
$GCC32 "${CFLAGS[@]}" -c "$STDLIB/string.c" -o "$BUILD_DIR/string.o" "-I$STDLIB" "-I$STDIO"
$GCC32 "${CFLAGS[@]}" -c "$STDLIB/math.c" -o "$BUILD_DIR/math.o" "-I$STDLIB" "-I$STDIO"
printf -- "\n\n\n======================== Special pointers ==============\n\n\n"
$GPP32 "${CPPFLAGS[@]}" -c "$STDLIB/special_pointers.cpp" -o "$BUILD_DIR/special_pointers.o" "-I$STDLIB" "-I$STDIO" -fno-strict-aliasing

# Compile Drivers CPU structures
$GCC32 "${CFLAGS_NOFPU[@]}" -c "$CPUID/cpuid.c" -o "$BUILD_DIR/cpuid_c.o"
$GPP32 "${CPPFLAGS_NOFPU[@]}" -c "$CPUID/cpuid.cpp" -o "$BUILD_DIR/cpuid.o" "-I$STDLIB" "-I$STDIO"
$GPP32 "${CPPFLAGS_NOFPU[@]}" -c "$FPU/fpu.cpp" -o "$BUILD_DIR/fpo.o" "-I$STDLIB" "-I$STDIO" "-I$CPUID" "-I$CONTROL_REGISTERS"

# Compile Drivers CPU structures
$GCC32 "${CFLAGS[@]}" -c "$GDT/f1_binary_operation.c" -o "$BUILD_DIR/f1_binary_operation.o" "-I$STDIO" "-I$GDT"
$GCC32 "${CFLAGS[@]}" -c "$GDT/f3_segment_descriptor_internals.c" -o "$BUILD_DIR/f3_segment_descriptor_internals.o" "-I$STDIO" "-I$GDT" "-I$STDLIB"
$GCC32 "${CFLAGS[@]}" -c "$GDT/gdt.c" -o "$BUILD_DIR/gdt.o" "-I$STDIO" "-I$GDT" "-I$STDLIB"
$GCC32 "${CFLAGS[@]}" -c "$IDT/idt.c" -o "$BUILD_DIR/idt.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$MULTICORE" "${PS2_SUPER_INCLUDE[@]}"
$GCC32 "${CFLAGS[@]}" -c "$IDT/idt_ps2.c" -o "$BUILD_DIR/idt_ps2.o" "-I$IDT" "-I$GDT" "-I$PIC" "-I$STDLIB" "-I$STDIO" "${PS2_SUPER_INCLUDE[@]}"
nasm "${NASM_FLAGS32[@]}" "$IDT/exception_handler.asm" -o "$BUILD_DIR/exception_handler.o"
$GCC32 "${CFLAGS[@]}" -c "$PIC/pic.c" -o "$BUILD_DIR/pic.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$CPU"

# CPU/APIC
printf -- "\n\n\n======================== APIC ==============\n\n\n"
$GPP32 "${CPPFLAGS[@]}" -c "$APIC/apic.cpp" -o "$BUILD_DIR/apic.o" "-I$STDLIB" "-I$STDIO" "-I$ACPI" "-I$CPUID" "-I$MULTICORE" "-I$CPU" "-I$PIT" "-I$GDT" "-I$APIC_IO"
$GPP32 "${CPPFLAGS[@]}" -c "$APIC_IO/apic_io.cpp" -o "$BUILD_DIR/apic_io.o" "-I$STDLIB" "-I$STDIO" "-I$ACPI" "-I$CPUID" "-I$MULTICORE" "-I$CPU" "-I$GDT"
$GPP32 "${CPPFLAGS[@]}" -c "$MULTICORE/multicore.cpp" -o "$BUILD_DIR/multicore.o" "-I$STDLIB" "-I$STDIO" "-I$APIC" "-I$CPUID" "-I$MULTICORE" "-I$CPU" "-I$GDT" "-I$FRAMEBUFER"
nasm "${NASM_FLAGS16[@]}" "$MULTICORE/multicore_bootstrap16.asm" -o "$BUILD_DIR/multicore_bootstrap16.o"
nasm "${NASM_FLAGS32[@]}" "$MULTICORE/multicore_bootstrap32.asm" -o "$BUILD_DIR/multicore_bootstrap32.o"

$GCC32 "${CFLAGS[@]}" -c "$MULTICORE/multicore.c" -o "$BUILD_DIR/multicore_c.o" "-I$STDLIB" "-I$APIC" "-I$MULTICORE" "-I$CPU"
# =============== Compile Drivers ==============

# PS2 Controller, Interrupt Handler And Enable Wrapper
nasm "${NASM_FLAGS32[@]}" "$PS2/ps2_interrupt_handlers.asm" -o "$BUILD_DIR/ps2_interrupt_handlers.o"
$GCC32 "${CFLAGS[@]}" -c "$PS2_CONTROLLER/ps2_controller.c" -o "$BUILD_DIR/ps2_controller.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$ACPI" "-I$DRIVERS_USB_CONTROLLER"
$GCC32 "${CFLAGS[@]}" -c "$PS2/ps2.c" -o "$BUILD_DIR/ps2.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "${PS2_SUPER_INCLUDE[@]}"
# PS2 Keyboard
$GCC32 "${CFLAGS[@]}" -c "$PS2_KEYBOARD/ps2_keyboard.c" -o "$BUILD_DIR/ps2_keyboard.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "${PS2_SUPER_INCLUDE[@]}"
$GCC32 "${CFLAGS[@]}" -c "$PS2_KEYBOARD/ps2_keyboard_handler.c" -o "$BUILD_DIR/ps2_keyboard_handler.o" \
	-I"$IDT" -I"$GDT" -I"$STDLIB" -I"$STDIO" "${PS2_SUPER_INCLUDE[@]}"
$GPP32 "${CPPFLAGS[@]}" -c "$PS2_KEYBOARD_CPP/keycodes.cpp" -o "$BUILD_DIR/keycodes.o" "-I$STDIO" "-I$STDLIB" "-I$PS2_KEYBOARD_CPP"
$GPP32 "${CPPFLAGS[@]}" -c "$PS2_KEYBOARD_CPP/scancodes_to_keycodes.cpp" -o "$BUILD_DIR/scancodes_to_keycodes.o" "-I$STDIO" "-I$STDLIB" "-I$PS2_KEYBOARD" "-I$PS2_KEYBOARD_CPP"
$GCC32 "${CFLAGS[@]}" -c "$PS2_KEYBOARD/ps2_keyboard_key_functions.c" -o "$BUILD_DIR/ps2_keyboard_key_functions.o" "-I$PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"
# PS2 Mouse
$GPP32 "${CPPFLAGS[@]}" -c "$PS2_MOUSE/ps2_mouse.cpp" -o "$BUILD_DIR/ps2_mouse.o" "-I$IDT" "-I$GDT" "-I$PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"
$GPP32 "${CPPFLAGS[@]}" -c "$PS2_MOUSE/ps2_mouse_handler.cpp" -o "$BUILD_DIR/ps2_mouse_handler.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "${PS2_SUPER_INCLUDE[@]}" "-I$FRAMEBUFER"

# Temporary stuff. Will properly program them one day.
$GCC32 "${CFLAGS[@]}" -c "$DRIVERS_USB_CONTROLLER/usb_controller.c" -o "$BUILD_DIR/usb_controller.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$DRIVERS_USB_CONTROLLER"
$GCC32 "${CFLAGS[@]}" -c "$OTHER/virtual_memory.c" -o "$BUILD_DIR/virtual_memory.o"

# Timers
$GPP32 "${CPPFLAGS[@]}" -c "$PIT/pit.cpp" -o "$BUILD_DIR/pit.o" "-I$STDLIB" "-I$STDIO" "-I$PIC" "-I$CPU"
$GCC32 "${CFLAGS[@]}" -c "$PIT/idt_pit.c" -o "$BUILD_DIR/idt_pit.o" "-I$STDLIB" "-I$STDIO" "-I$PIC" "-I$CPU" "-I$IDT" "-I$GDT"
$GPP32 "${CPPFLAGS[@]}" -c "$PIT/pit_interrupt_handler.cpp" -o "$BUILD_DIR/pit_interrupt_handler.o" "-I$STDLIB" "-I$STDIO" "-I$PIC" "-I$CPU" "-I$APIC"
nasm "${NASM_FLAGS32[@]}" "$PIT/pit_interrupt_handler.asm" -o "$BUILD_DIR/pit_interrupt_handler_asm.o"

$GPP32 "${CPPFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$FRAMEBUFER/framebuffer.cpp" -o "$BUILD_DIR/framebuffer.o"

# Super Misc
$GCC32 "${CFLAGS[@]}" -c "$CODE_ANALYSIS/address_getter.c" -o "$BUILD_DIR/address_getter.o" "-I$REAL_FUNC" "-I$REAL16_WRAPPERS" "-I$GDT" "-I$STDLIB"

# Compile the real mode 16 code and it's wrappers
$GCC32 "${CFLAGS[@]}" -c "$REAL16_WRAPPERS/call_real16_wrapper.c" -o "$BUILD_DIR/call_real16_wrapper.o" "-I$STDIO" "-I$STDLIB" "-I$GDT"
nasm "${NASM_FLAGS16[@]}" "$REAL16_WRAPPERS/call_realmode_function_wrapper16.asm" -o "$BUILD_DIR/call_realmode_function_wrapper16.o" "-I$REAL16_WRAPPERS"
nasm "${NASM_FLAGS32[@]}" "$REAL16_WRAPPERS/call_realmode_function_wrapper32.asm" -o "$BUILD_DIR/call_realmode_function_wrapper32.o" "-I$REAL16_WRAPPERS"
ia16-elf-gcc "${CFLAGS16[@]}" -c "$REAL_FUNC/realmode_functions.c" -o "$BUILD_DIR/realmode_functions.o"

# Compile Other language projects (Library written entirely in ~(C or ASM))

printf "\n\n============Start of Module Build ============\n\n"
./modules/build.sh

printf "\n\n============Start of Zig Build ============\n\n"
./z_otherLang/zig/build.sh

printf "\n\n============Start of Rust Build ============\n\n"
./z_otherLang/rust/build.sh

# Link the kernel and generate the final binary

# fails
BUILD_OBJECTS=("$BUILD_DIR"/*.o)
if true; then
	# BUILD_OBJECTS+=("$BUILD_DIR"/banana/*.o) # banana subdir
	# BUILD_OBJECTS+=("$BUILD_DIR"/apple/*.o)  # apple subdir
	BUILD_OBJECTS+=("$MODULES"/build/*.o) # modules build object subdir
fi

# ========= Static library setup ============
printf "\n\n====== Start of Static Library Setup =====\n\n"

# Library configuration
LIBRARY_PATHS=(
	"$BUILD_DIR"
	# "/path/to/other/libs"
)

LIBRARY_FILES=(
	# "libkernel_zig.a"
	# "libother.a"
)

# Build the -L and library arguments
LIBRARY_ARGS=()
for path in "${LIBRARY_PATHS[@]}"; do
	LIBRARY_ARGS+=("-L$path")
done

for lib in "${LIBRARY_FILES[@]}"; do
	LIBRARY_ARGS+=("$lib")
done

# ============= Linking ==============
printf "\n\n====== Start of Linking =====\n\n"
# could also use g++. But as long as no runtime support, gcc will work
$GPP32 -T linker32.ld -o "$BUILD_DIR/kernel32.elf" "${LDFLAGS[@]}" "${BUILD_OBJECTS[@]}" "${LIBRARY_ARGS[@]}"

printf "\n\n====== End of Linking =====\n\n"

objdump -D -h -M intel "$BUILD_DIR/kernel32.elf" >"$BUILD_DIR/myos.dump"

objdump -D -h -M intel "$BUILD_DIR/apic.o" >"$BUILD_DIR/apic.dump"
objdump -D -h -M intel "$BUILD_DIR/multicore_c.o" >"$BUILD_DIR/multicore_c.dump"
objdump -D -h -M intel "$BUILD_DIR/special_pointers.o" >"$BUILD_DIR/special_pointers.dump"

cp "$BUILD_DIR/kernel32.elf" "$ISO_DIR/boot/kernel32.elf"
printf "\n\n====== End of Build32.sh =====\n\n"

# qemu-system-i386 -kernel ./build/kernel32.elf & # or do this to use the binary directly # -cdrom "$BUILD_DIR/myos.iso" # -kernel "$BUILD_DIR/kernel32.elf" \
