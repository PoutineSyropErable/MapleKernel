#!/usr/bin/env bash

set -eou pipefail

BUILD_DIR="build"

BINARY_OPERATION="f1_binary_operation16"
STRING="f2_string16"
GDT_WRAPPERS="f3_gdt_wrappers16"
GDT_WRAPPERS_EXTENSION="f3_1_gdt_wrappers_extension_32"
MAIN="f4_main16" # updated to match your original main file

OUT="gdt16"

mkdir -p "$BUILD_DIR"

# Compile each source file
gcc -c "$BINARY_OPERATION.c" -o "$BUILD_DIR/$BINARY_OPERATION.o"
gcc -c "$STRING.c" -o "$BUILD_DIR/$STRING.o"
gcc -c "$GDT_WRAPPERS.c" -o "$BUILD_DIR/$GDT_WRAPPERS.o"
gcc -c "$GDT_WRAPPERS_EXTENSION.c" -o "$BUILD_DIR/$GDT_WRAPPERS_EXTENSION.o"
gcc -c "$MAIN.c" -o "$BUILD_DIR/$MAIN.o"

# Link object files into final executable
gcc "$BUILD_DIR/$BINARY_OPERATION.o" \
	"$BUILD_DIR/$STRING.o" \
	"$BUILD_DIR/$GDT_WRAPPERS.o" \
	"$BUILD_DIR/$GDT_WRAPPERS_EXTENSION.o" \
	"$BUILD_DIR/$MAIN.o" \
	-o "$OUT"

echo
./$OUT
