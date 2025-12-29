#include "assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "symbols.h"
#include <stddef.h>
// Linker provides these
extern char __ksymtab_start[];
extern char __ksymtab_end[];
extern char __kstrtab_start[];
extern char __kstrtab_end[];

// For 32-bit ELF
typedef struct
{
    uint32_t      name;  // String table index (4 bytes)
    uint32_t      value; // Symbol value/address (4 bytes)
    uint32_t      size;  // Size of symbol (4 bytes)
    unsigned char info;  // Type and binding (1 byte)
    unsigned char other; // Visibility (1 byte)
    uint16_t      shndx; // Section index (2 bytes)
} __attribute__((packed)) Elf32_Sym;

static Elf32_Sym *elf_symtab   = (Elf32_Sym *)__ksymtab_start;
static char      *elf_strtab   = __kstrtab_start;
static int        elf_symcount = 0;

// uint32_t abs(int x)
// {
//     if (x < 0)
//         return -x;
//     else
//         return x;
// }
//
// bool is_close(uint32_t a, uint32_t b, uint32_t diff_allowed)
// {
//
//     int      diff   = ((int)a - (int)b);
//     uint32_t diff_p = abs(diff);
//     return diff_p < diff_allowed;
// }

_Static_assert(sizeof(Elf32_Sym) == 16, "Elf32_Sym must be 16 byte\n");

void init_elf_symbols(void)
{
    elf_symcount = (__ksymtab_end - __ksymtab_start) / sizeof(Elf32_Sym);

    // MUST be divisible by 16!
    if (((__ksymtab_end - __ksymtab_start) % 16) != 0)
    {
        kprintf("ERROR: Symbol table size not multiple of 16!\n");
        kprintf("Size mod 16 = %d\n", (__ksymtab_end - __ksymtab_start) % 16);
        abort();
    }

    kprintf("ELF symbols: %d entries\n", elf_symcount);

    // Optional: print some symbols
    for (int i = 0; i < elf_symcount; i++)
    {
        // kprintf("name = %s, address =%h\n", elf_strtab + elf_symtab[i].name, elf_symtab[i].value);
        // if (is_close(elf_symtab[i].value, 9828, 50))
        // {
        //     kprintf("name = %s, address =%h\n", elf_strtab + elf_symtab[i].name, elf_symtab[i].value);
        // }
    }
}

uintptr_t find_symbol_address(const char *name)
{
    if (!elf_symtab)
        init_elf_symbols();

    for (int i = 0; i < elf_symcount; i++)
    {
        const char *symname = elf_strtab + elf_symtab[i].name;
        const char *n       = name;
        while (*symname && *symname == *n)
        {
            symname++;
            n++;
        }
        if (*symname == *n)
            return elf_symtab[i].value;
    }
    return 0;
}

const char *find_symbol_name(uintptr_t address)
{
    if (!elf_symtab)
        init_elf_symbols();

    for (int i = 0; i < elf_symcount; i++)
    {
        if (elf_symtab[i].value == address)
        {
            return elf_strtab + elf_symtab[i].name;
        }
    }
    return NULL;
}

// Find the nearest symbol <= address (for stack traces)
const char *find_nearest_symbol(uintptr_t address, uintptr_t *offset)
{
    if (!elf_symtab)
        init_elf_symbols();

    const char *best_name = NULL;
    uintptr_t   best_addr = 0;

    for (int i = 0; i < elf_symcount; i++)
    {
        uintptr_t sym_addr = elf_symtab[i].value;
        if (sym_addr != 0 && sym_addr <= address)
        {
            if (sym_addr > best_addr)
            {
                best_addr = sym_addr;
                best_name = elf_strtab + elf_symtab[i].name;
            }
        }
    }

    if (best_name && offset)
    {
        *offset = address - best_addr;
    }

    return best_name;
}
