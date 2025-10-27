#!/usr/bin/env bash
# Compile 16-bit assembly function
gcc -m32 -c add16.c -o add16.o

# Compile 32-bit main
gcc -m32 -c main32.c -o main32.o

# Link together
gcc -m32 main32.o add16.o -o hybrid32
