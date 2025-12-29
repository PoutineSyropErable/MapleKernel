#!/usr/bin/env bash

set -eou pipefail

# Build 32-bit kernel + trampoline
nasm -f elf32 boot_intel.asm -o boot.o
i686-elf-gcc -ffreestanding -m32 -c kernel32.c -o kernel32.o
nasm -f elf32 trampoline32.asm -o trampoline32.o
nasm -f elf64 kernel_caller.asm -o kernel_caller.o

echo "first link"
# ld -m elf_i386 -T linker32.ld kernel32.o trampoline32.o kernel_caller.o boot.o -o kernel32.elf

x86_64-elf-gcc -ffreestanding -nostdlib -lgcc \
	-T linker32.ld kernel32.o trampoline32.o kernel_caller.o boot.o \
	-o kernel32.elf

echo "second link"

# Build 64-bit kernel
x86_64-elf-gcc -ffreestanding -m64 -c kernel64.c -o kernel64.o
ld -m elf_x86_64 -T linker64.ld kernel_caller.o kernel64.o -o kernel64.elf
