#!/usr/bin/env bash

set -eou pipefail

BUILD_DIR="build"

BINARY_OPERATION="f1_binary_operation32"
STRING="f2_string32"
SEGMENT_DESCRIPTOR="f3_segment_descriptor_internals32"
MAIN="f4_main" # updated to match your original main file

mkdir -p "$BUILD_DIR"

# Compile each source file
gcc -c "$BINARY_OPERATION.c" -o "$BUILD_DIR/binary_operation.o"
gcc -c "$STRING.c" -o "$BUILD_DIR/string.o"
gcc -c "$SEGMENT_DESCRIPTOR.c" -o "$BUILD_DIR/segment_descriptor_internals32.o"
gcc -c "$MAIN.c" -o "$BUILD_DIR/main.o"

# Link object files into final executable
gcc "$BUILD_DIR/binary_operation.o" \
	"$BUILD_DIR/string.o" \
	"$BUILD_DIR/segment_descriptor_internals32.o" \
	"$BUILD_DIR/main.o" \
	-o segment_descriptors

echo
./segment_descriptors
