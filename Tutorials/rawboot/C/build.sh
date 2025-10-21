#!/usr/bin/env bash

set -eou pipefail

mkdir -p build

gcc -c f1_binary_operation.c -o build/binary_operation.o
gcc -c f2_string.c -o build/string.o
gcc -c f3_segment_descriptor_internals.c -o build/segment_descriptor_internals.o
gcc -c f4_main.c -o build/main.o

# using ld rather then gcc will fuck calls to libc
gcc build/binary_operation.o build/string.o build/segment_descriptor_internals.o build/main.o -o segment_descriptors

echo
./segment_descriptors
