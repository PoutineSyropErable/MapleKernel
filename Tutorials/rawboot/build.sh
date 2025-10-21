#!/usr/bin/env bash
set -euo pipefail
alias ld64="x86_64-elf-ld"

mkdir -p build

# Assemble 16-bit stages
nasm -f bin -o build/boot16.bin boot16.s
nasm -f bin -o build/boot16_2.bin boot16_2.s

# Assemble 32-bit stage (NASM)
nasm -f elf32 -o build/boot32.o boot32.s

# Assemble 64-bit stage (NASM)
nasm -f elf64 -o build/boot64.o boot64.s

# Link everything into a flat binary
ld64 -T linker.ld -o build/mykernel.bin \
	build/boot16.o build/boot16_2.o build/boot32.o build/boot64.o

echo "Build complete. Output: build/mykernel.bin"
