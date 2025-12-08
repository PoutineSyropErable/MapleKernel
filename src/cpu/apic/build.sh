#!/usr/bin/env bash

BUILD_DIR="../../build/"

CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra")
CPPFLAGS=("-std=gnu++23" "-ffreestanding" "-Wall" "-Wextra")
CFLAGS16=("-std=gnu99" "-ffreestanding" "-Wall" "-Wextra")
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc")
NASM_FLAGS32=("-f" "elf32")
NASM_FLAGS16=("-f" "elf")

DEBUG_OPT_LVL="-O0"
RELEASE_OPT_LVL="-O3"

if [[ "$DEBUG_NOT_RELEASE" == "debug" ]]; then
	echo "Debug mode enabled"
	CFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	CPPFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	CFLAGS16+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	LDFLAGS+=("-g")
	NASM_FLAGS32+=("-g" "-F" "dwarf")
	NASM_FLAGS16+=("-g" "-F" "dwarf")
	QEMU_DBG_FLAGS+=("-s" "-S")
else
	echo "In normal mode, $RELEASE_OPT_LVL optimisation"
	CFLAGS+=("$RELEASE_OPT_LVL")
	CPPFLAGS+=("$RELEASE_OPT_LVL")
	CFLAGS16+=("$RELEASE_OPT_LVL")
	LDFLAGS+=("$RELEASE_OPT_LVL")
fi

KERNEL="../../kernel"
REAL16_WRAPPERS="../../real16_wrappers"
REAL_FUNC="../../realmode_functions"
GDT="../../gdt"
IDT="../../idt"
STDIO="../../stdio"
STDLIB="../../stdlib"
OTHER="../../other"
CODE_ANALYSIS="../../runtime_code_analysis"

# Unused. but moved stuff there eventually
DRIVERS="../../drivers"
DRIVERS_PS2="../../drivers/ps2"
DRIVERS_PS2_CONTROLLER="../../drivers/ps2/controller"
DRIVERS_PS2_KEYBOARD="../../drivers/ps2/keyboard"
DRIVERS_PS2_MOUSE="../../drivers/ps2/mouse"
DRIVERS_PS2_KEYBOARD_CPP="../../drivers/ps2/keyboard/cpp"

DRIVERS_USB_CONTROLLER="../../drivers/usb/controller"
ACPI="../../acpi"

# ============ The new stuff
MADT="./madt"
APIC="./"

i686-elf-g++ "${CPPFLAGS[@]}" -c "$MADT/madt.cpp" -o "$BUILD_DIR/madt.o" "-I$STDIO" "-I$STDLIB"
