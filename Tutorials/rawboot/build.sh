#!/usr/bin/env bash

# 16-bit boot sector / reset vector
nasm -f bin boot16.s -o boot16.bin

# 16-bit real mode setup
nasm -f elf32 boot16_2.s -o boot16_2.o

# 32-bit protected mode
nasm -f elf32 boot32.s -o boot32.o

# 64-bit long mode
nasm -f elf64 boot64.s -o boot64.o

ld -T linker.ld -o mykernel.bin boot16_2.o boot32.o boot64.o

qemu-system-x86_64 \
	-machine accel=tcg \
	-nographic \
	-bios none \
	-device loader,file=mykernel.bin,addr=0xFFFFFFF0
