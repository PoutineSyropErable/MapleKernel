#!/usr/bin/env bash

set -eou pipefail
DEBUG_OR_RELEASE="${1:-release}"
QEMU_OR_REAL_MACHINE="${2:-qemu}"
MACHINE_BITNESS="${3:-32}"
MOVE_VNC="${4:-move}"

# Check for help argument
if [[ "${1:-}" == "help" ]]; then
	cat <<'EOF'
Usage: ./build.sh [debug|release] [QEMU|REAL] [32|64] [move|nomove]

Arguments:
  debug|release       Build mode. Defaults to release if omitted.
  qemu|real           Whether to run in QEMU or on a real machine. Defaults to QEMU.
  32|64               Machine bitness. Defaults to 32.
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
src="$project_root/src"
cd "$src" || { # Tries to cd to "src" relative to current dir
	echo "Could not cd to $src."
	exit 1
}

#With this, it should work nice
export PATH="./tools:$PATH"
source ./tools/source_this.bash

# Define directories
BUILD_DIR="build"
ISO_DIR="isodir"

CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra")
CPPFLAGS=("-std=gnu++23" "-ffreestanding" "-Wall" "-Wextra")
CFLAGS16=("-std=gnu99" "-ffreestanding" "-Wall" "-Wextra")
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc" "-fno-eliminate-unused-debug-symbols")
NASM_FLAGS32=("-f" "elf32")
NASM_FLAGS16=("-f" "elf")

DEBUG_OPT_LVL="-O0"
RELEASE_OPT_LVL="-O0"
QEMU_DBG_FLAGS=()

if [[ "$DEBUG_OR_RELEASE" == "debug" ]]; then
	echo "Debug mode enabled"
	CFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	CPPFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
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

# Create necessary directories
mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

# The variable for the dirs:
KERNEL="kernel"
REAL16_WRAPPERS="real16_wrappers"
REAL_FUNC="realmode_functions"
STDIO="stdio"
STDLIB="stdlib"
OTHER="other"
CODE_ANALYSIS="runtime_code_analysis"

# Unused. but moved stuff there eventually
DRIVERS="./drivers"
DRIVERS_PS2="./drivers/ps2"
DRIVERS_PS2_CONTROLLER="./drivers/ps2/controller"
DRIVERS_PS2_KEYBOARD="./drivers/ps2/keyboard"
DRIVERS_PS2_MOUSE="./drivers/ps2/mouse"
DRIVERS_PS2_KEYBOARD_CPP="./drivers/ps2/keyboard/cpp"

DRIVERS_USB_CONTROLLER="./drivers/usb/controller"

UEFI="./firmware/uefi"
MULTIBOOT="./firmware/multiboot"
ACPI="./firmware/acpi"

APIC="./cpu/apic"
GDT="./cpu/gdt"
IDT="./cpu/idt"

CPP="./z_otherLang/cpp/"
RUST="./z_otherLang/rust/"
ZIG="./z_otherLang/zig/"

FRAMEBUFER="./framebuffer"

INCLUDE_DIRS=(
	"$KERNEL"
	"$REAL16_WRAPPERS"
	"$REAL_FUNC"
	"$GDT"
	"$IDT"
	"$STDIO"
	"$STDLIB"
	"$OTHER"
	"$CODE_ANALYSIS"

	"$DRIVERS_PS2_CONTROLLER"
	"$DRIVERS_PS2_KEYBOARD"
	"$DRIVERS_PS2_KEYBOARD_CPP"
	"$DRIVERS_PS2_MOUSE"

	"$DRIVERS_USB_CONTROLLER"

	"$UEFI"
	"$MULTIBOOT"
	"$APIC"
	"$ACPI"

	"$FRAMEBUFER"

	"$CPP"
	"$RUST"
	"$ZIG"
)

PS2_INCLUDE_DIRS=(
	"$DRIVERS_PS2"
	"$DRIVERS_PS2_CONTROLLER"
	"$DRIVERS_PS2_KEYBOARD"
	"$DRIVERS_PS2_MOUSE"
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

# Assemble the bootloader assembly
nasm "${NASM_FLAGS32[@]}" "$KERNEL/boot_intel.asm" -o "$BUILD_DIR/boot.o"
i686-elf-gcc "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$KERNEL/kernel.c" -o "$BUILD_DIR/kernel.o"
i686-elf-gcc "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$KERNEL/symbols.c" -o "$BUILD_DIR/symbols.o"
i686-elf-gcc "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$KERNEL/kernel_helper.c" -o "$BUILD_DIR/kernel_helper.o"

# Move this to firmware maybe?
i686-elf-g++ "${CPPFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$MULTIBOOT/multiboot.cpp" -o "$BUILD_DIR/multiboot.o"
i686-elf-g++ "${CPPFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$FRAMEBUFER/framebuffer.cpp" -o "$BUILD_DIR/framebuffer.o"

# Compile the print functions.
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/string_helper.c" -o "$BUILD_DIR/string_helper.o"
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/bit_hex_string.c" -o "$BUILD_DIR/bit_hex_string.o" -std=gnu99 "-I$STDIO" "-I$STDLIB"
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/vga_terminal.c" -o "$BUILD_DIR/vga_terminal.o" "-I$STDIO" "-I$OTHER" "-I$STDLIB"
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/stdio.c" -o "$BUILD_DIR/stdio.o" "-I$STDIO" "-I$OTHER" "-I$STDLIB"

#compile misc helper functions
i686-elf-gcc "${CFLAGS[@]}" -c "$STDLIB/stdlib.c" -o "$BUILD_DIR/stdlib.o" "-I$STDLIB" "-I$STDIO"

i686-elf-gcc "${CFLAGS[@]}" -c "$GDT/f1_binary_operation.c" -o "$BUILD_DIR/f1_binary_operation.o" "-I$STDIO" "-I$GDT"
i686-elf-gcc "${CFLAGS[@]}" -c "$GDT/f3_segment_descriptor_internals.c" -o "$BUILD_DIR/f3_segment_descriptor_internals.o" "-I$STDIO" "-I$GDT" "-I$STDLIB"
i686-elf-gcc "${CFLAGS[@]}" -c "$GDT/gdt.c" -o "$BUILD_DIR/gdt.o" "-I$STDIO" "-I$GDT" "-I$STDLIB"

# Compile Drivers CPU structures
i686-elf-gcc "${CFLAGS[@]}" -c "$IDT/idt.c" -o "$BUILD_DIR/idt.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "${PS2_SUPER_INCLUDE[@]}"
i686-elf-gcc "${CFLAGS[@]}" -c "$IDT/idt_ps2.c" -o "$BUILD_DIR/idt_ps2.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "${PS2_SUPER_INCLUDE[@]}"
i686-elf-gcc "${CFLAGS[@]}" -c "$IDT/pic.c" -o "$BUILD_DIR/pic.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO"
nasm "${NASM_FLAGS32[@]}" "$IDT/exception_handler.asm" -o "$BUILD_DIR/exception_handler.o"

# Compile Drivers
nasm "${NASM_FLAGS32[@]}" "$DRIVERS_PS2/ps2_interrupt_handlers.asm" -o "$BUILD_DIR/ps2_interrupt_handlers.o"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_CONTROLLER/ps2_controller.c" -o "$BUILD_DIR/ps2_controller.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$ACPI" "-I$DRIVERS_USB_CONTROLLER"

i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_KEYBOARD/ps2_keyboard.c" -o "$BUILD_DIR/ps2_keyboard.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_KEYBOARD/ps2_keyboard_handler.c" -o "$BUILD_DIR/ps2_keyboard_handler.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO" "-I$DRIVERS_PS2_KEYBOARD_CPP"
i686-elf-g++ "${CPPFLAGS[@]}" -c "$DRIVERS_PS2_KEYBOARD_CPP/keycodes.cpp" -o "$BUILD_DIR/keycodes.o" "-I$STDIO" "-I$STDLIB" "-I$DRIVERS_PS2_KEYBOARD_CPP"
i686-elf-g++ "${CPPFLAGS[@]}" -c "$DRIVERS_PS2_KEYBOARD_CPP/scancodes_to_keycodes.cpp" -o "$BUILD_DIR/scancodes_to_keycodes.o" "-I$STDIO" "-I$STDLIB" "-I$DRIVERS_PS2_KEYBOARD" "-I$DRIVERS_PS2_KEYBOARD_CPP"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_KEYBOARD/ps2_keyboard_key_functions.c" -o "$BUILD_DIR/ps2_keyboard_key_functions.o" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"

i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_MOUSE/ps2_mouse.c" -o "$BUILD_DIR/ps2_mouse.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_MOUSE/ps2_mouse_handler.c" -o "$BUILD_DIR/ps2_mouse_handler.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"

# Temporary stuff. Will properly program them one day.
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_USB_CONTROLLER/usb_controller.c" -o "$BUILD_DIR/usb_controller.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$DRIVERS_USB_CONTROLLER"
i686-elf-gcc "${CFLAGS[@]}" -c "$ACPI/acpi.c" -o "$BUILD_DIR/acpi.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$ACPI"
i686-elf-gcc "${CFLAGS[@]}" -c "$OTHER/virtual_memory.c" -o "$BUILD_DIR/virtual_memory.o"
i686-elf-gcc "${CFLAGS[@]}" -c "$OTHER/pit_timer.c" -o "$BUILD_DIR/pit_timer.o"

# Just another thing
i686-elf-gcc "${CFLAGS[@]}" -c "$CODE_ANALYSIS/address_getter.c" -o "$BUILD_DIR/address_getter.o" "-I$REAL_FUNC" "-I$REAL16_WRAPPERS" "-I$GDT" "-I$STDLIB"

# Compile the real mode 16 code and it's wrappers
i686-elf-gcc "${CFLAGS[@]}" -c "$REAL16_WRAPPERS/call_real16_wrapper.c" -o "$BUILD_DIR/call_real16_wrapper.o" "-I$STDIO" "-I$STDLIB" "-I$GDT"
nasm "${NASM_FLAGS16[@]}" "$REAL16_WRAPPERS/call_realmode_function_wrapper16.asm" -o "$BUILD_DIR/call_realmode_function_wrapper16.o" "-I$REAL16_WRAPPERS"
nasm "${NASM_FLAGS32[@]}" "$REAL16_WRAPPERS/call_realmode_function_wrapper32.asm" -o "$BUILD_DIR/call_realmode_function_wrapper32.o" "-I$REAL16_WRAPPERS"
ia16-elf-gcc "${CFLAGS16[@]}" -c "$REAL_FUNC/realmode_functions.c" -o "$BUILD_DIR/realmode_functions.o"

# Compile other languages files
i686-elf-g++ "${CPPFLAGS[@]}" -c "$CPP/kernel_cpp.cpp" -o "$BUILD_DIR/kernel_cpp.o" "-I$STDIO" "-I$STDLIB"

printf "\n\n============Start of Zig Build ============\n\n"
./z_otherLang/zig/build.sh

printf "\n\n============Start of Rust Build ============\n\n"
./z_otherLang/rust/build.sh

# Link the kernel and generate the final binary
printf "\n\n====== Start of Linking =====\n\n"

BUILD_OBJECTS=(
	"$BUILD_DIR/boot.o"
	"$BUILD_DIR/kernel.o"
	"$BUILD_DIR/symbols.o"
	"$BUILD_DIR/kernel_helper.o"
	"$BUILD_DIR/multiboot.o"

	"$BUILD_DIR/stdlib.o"

	"$BUILD_DIR/address_getter.o"

	"$BUILD_DIR/string_helper.o"
	"$BUILD_DIR/bit_hex_string.o"
	"$BUILD_DIR/vga_terminal.o"
	"$BUILD_DIR/stdio.o"

	"$BUILD_DIR/exception_handler.o"
	"$BUILD_DIR/idt.o"
	"$BUILD_DIR/idt_ps2.o"
	"$BUILD_DIR/pic.o"
	"$BUILD_DIR/ps2_controller.o"
	"$BUILD_DIR/ps2_keyboard.o"
	"$BUILD_DIR/keycodes.o"
	"$BUILD_DIR/scancodes_to_keycodes.o"
	"$BUILD_DIR/ps2_mouse.o"
	"$BUILD_DIR/ps2_keyboard_handler.o"
	"$BUILD_DIR/ps2_keyboard_key_functions.o"
	"$BUILD_DIR/ps2_mouse_handler.o"
	"$BUILD_DIR/ps2_interrupt_handlers.o"

	"$BUILD_DIR/usb_controller.o"
	"$BUILD_DIR/acpi.o"
	"$BUILD_DIR/virtual_memory.o"
	"$BUILD_DIR/pit_timer.o"

	"$BUILD_DIR/f1_binary_operation.o"
	"$BUILD_DIR/f3_segment_descriptor_internals.o"
	"$BUILD_DIR/gdt.o"

	"$BUILD_DIR/call_realmode_function_wrapper32.o"
	"$BUILD_DIR/call_realmode_function_wrapper16.o"
	"$BUILD_DIR/realmode_functions.o"
	"$BUILD_DIR/call_real16_wrapper.o"

	"$BUILD_DIR/kernel_cpp.o"
	"$BUILD_DIR/kernel_zig.o"

	"$BUILD_DIR/framebuffer.o"
)

# ========= Static library setup ============

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
# could also use g++. But as long as no runtime support, gcc will work
i686-elf-g++ -T linker.ld -o "$BUILD_DIR/myos_s.elf" "${LDFLAGS[@]}" "${BUILD_OBJECTS[@]}" "${LIBRARY_ARGS[@]}"

printf "\n\n====== Copying Symbol Table =====\n\n"

# Use readelf to dump raw section data
readelf -x .symtab build/myos_s.elf | tail -n +3 | xxd -r -p >build/symtab_raw.bin
readelf -x .strtab build/myos_s.elf | tail -n +3 | xxd -r -p >build/strtab_raw.bin

# Verify
echo "File sizes:"
ls -la build/symtab_raw.bin build/strtab_raw.bin
echo "First 16 bytes of symtab_raw.bin:"
# hexdump -C build/symtab_raw.bin
hexdump -C build/strtab_raw.bin

# 3. Assemble (need to be in src directory)
i686-elf-as -o "$BUILD_DIR/copy_symbols.o" "copy_symbols.S"

# 4. Link with copied symbols
i686-elf-g++ -T linker.ld -o "$BUILD_DIR/myos.elf" \
	"${LDFLAGS[@]}" "${BUILD_OBJECTS[@]}" "$BUILD_DIR/copy_symbols.o" "${LIBRARY_ARGS[@]}"

printf "\n\n====== End of Linking =====\n\n"

# objdump -D -h "$BUILD_DIR/myos.elf" >"$BUILD_DIR/myos.dump"

# Check if the kernel is multiboot-compliant
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

# Copy the kernel binary and GRUB configuration to the ISO directory
cp "$BUILD_DIR/myos.elf" "$ISO_DIR/boot/myos.elf"
cp "$ISO_DIR/grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"

# Create the ISO image
grub-mkrescue -o "$BUILD_DIR/myos.iso" "$ISO_DIR"
echo "ISO created successfully: $BUILD_DIR/myos.iso"

# Check if Ventoy USB is mounted
VENTOY_PATH="/run/media/$USER/Ventoy"
if [ -d "$VENTOY_PATH" ]; then
	echo "Ventoy detected at $VENTOY_PATH. Copying ISO..."
	cp "$BUILD_DIR/myos.iso" "$VENTOY_PATH/"
	echo "ISO copied to Ventoy USB successfully."
else
	echo "Ventoy USB not found at $VENTOY_PATH. Skipping copy."
fi

# Create 64 MB raw disk image

# start qemu using the binary, bypassing grub.
# -no-reboot: don't reset if the kernel crash
# -d in_asm, int, cpu_reset: Logs every instruction executed by cpu, every interupt and cpu reset events
# -D qemu_instr.log : The log file (redirect -d)
# -serial stdio: redirect COM1 serial port to your terminal

# ===== Pick one of those two
# -kernel "$BUILD_DIR/myos.elf" \
# -cdrom "$BUILD_DIR/myos.iso" \

if [[ "$QEMU_OR_REAL_MACHINE" != "qemu" ]]; then
	printf -- "\nSince Not In Qemu, then we aren't gonna emulate the machine\n\n"
	exit 1
fi

USE_IMAGE=false
if [ "$USE_IMAGE" == true ]; then
	bochs -f bochsrc.txt
else
	QEMU32=qemu-system-i386
	QEMU64=qemu-system-x86_64

	# Choose QEMU based on machine bitness
	if [[ "$MACHINE_BITNESS" == "64" ]]; then
		QEMU="$QEMU64"
	else
		QEMU="$QEMU32"
	fi

	# Check if QEMU is already running, kill only if it exists
	if pgrep -f "$QEMU" >/dev/null 2>&1; then
		echo "Killing existing QEMU process..."
		pkill -f "$QEMU" 2>/dev/null
		sleep 1 # Give it time to die
	fi

	$QEMU \
		-cdrom "$BUILD_DIR/myos.iso" \
		\
		-no-reboot \
		"${QEMU_DBG_FLAGS[@]}" \
		-d in_asm,int,cpu_reset \
		-D qemu_instr.log \
		-serial stdio & # -vga vmware \

	QEMU_PID=$!

	# Give QEMU a second to start up
	sleep 1

	# Launch VNC viewer
	vncviewer localhost:5900 &
	VNC_PID=$!

	# sleep 1
	if [[ "$MOVE_VNC" == "move" ]]; then
		move_pid_to_workspace $VNC_PID 21
	fi

	wait $VNC_PID

	# After you close the VNC viewer, kill QEMU
	kill $QEMU_PID 2>/dev/null
fi
# qemu-system-i386 -kernel ./build/myos.elf & # or do this to use the binary directly # -cdrom "$BUILD_DIR/myos.iso" # -kernel "$BUILD_DIR/myos.elf" \
