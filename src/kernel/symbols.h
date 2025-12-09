#pragma once
#include <stdint.h>

struct kernel_symbol
{
    const char *name;
    uintptr_t   address;
};

// ELF-based functions
void        init_elf_symbols(void);
uintptr_t   find_symbol_address(const char *name);
const char *find_symbol_name(uintptr_t address);
const char *find_nearest_symbol(uintptr_t address, uintptr_t *offset);
