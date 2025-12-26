#!/usr/bin/env bash

set -eou pipefail
shopt -s nullglob

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

	"$EFI"
	"$UEFI"
	"$MULTIBOOT"
	"$ACPI"
	"$ACPI/madt"

	"$FRAMEBUFER"
	"$PIT"

	"$KERNEL_CPP"
	"$RUST"
	"$ZIG"
	"$MODULES"
)

# have 1 file which include every .h and .hpp, then it will be find
clang-query -p . kernel.c -c 'match fieldDecl(hasType(pointerType()), hasParent(recordDecl(isStruct())))'
