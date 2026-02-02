#!/usr/bin/env bash

${CC:-gcc} -Wall -Wextra -Werror -no-pie -O0 -g -gdwarf-"${DWARF:-5}" -o cloop main.c

# Then parse it
go run elf_parser.go cloop
