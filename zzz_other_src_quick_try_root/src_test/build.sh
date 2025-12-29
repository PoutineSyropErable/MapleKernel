#!/usr/bin/env bash

BUILD_DIR="./build"
mkdir -p "$BUILD_DIR"

for f in *.c; do
	printf -- "-----The current file: %s\n\n" "$f"
	gcc -c "$f" -o "$BUILD_DIR/${f%.c}.o" -g
	printf -- "\n---End of build---\n"
done

gcc "$BUILD_DIR/test.o" "$BUILD_DIR/stdio.o" "$BUILD_DIR/string_helper.o" -o test

./test
