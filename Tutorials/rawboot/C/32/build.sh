#!/usr/bin/env bash

set -eou pipefail

BUILD_DIR="build"

BINARY_OPERATION="f1_binary_operation32"
STRING="f2_string32"
SEGMENT_DESCRIPTOR="f3_segment_descriptor_internals32"
MAIN="f4_main32" # updated to match your original main file

OUT="gdt32"

mkdir -p "$BUILD_DIR"

# Compile each source file
gcc -c "$BINARY_OPERATION.c" -o "$BUILD_DIR/$BINARY_OPERATION.o"
gcc -c "$STRING.c" -o "$BUILD_DIR/$STRING.o"
gcc -c "$SEGMENT_DESCRIPTOR.c" -o "$BUILD_DIR/$SEGMENT_DESCRIPTOR.o"
gcc -c "$MAIN.c" -o "$BUILD_DIR/$MAIN.o"

# Link object files into final executable
gcc "$BUILD_DIR/$BINARY_OPERATION.o" \
	"$BUILD_DIR/$STRING.o" \
	"$BUILD_DIR/$SEGMENT_DESCRIPTOR.o" \
	"$BUILD_DIR/$MAIN.o" \
	-o "$OUT"

echo
./$OUT
