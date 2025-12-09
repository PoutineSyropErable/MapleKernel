#include "stdio.h"
#include "symbols.h"
#include <stddef.h>

// Linker provides these
extern char __ksymtab_start[];
extern char __ksymtab_end[];
extern char __kstrtab_start[];
extern char __kstrtab_end[];

typedef struct
{
    uint32_t  name;  // Offset in .strtab
    uintptr_t value; // Address
    uint32_t  size;
    uint8_t   info;
    uint8_t   other;
    uint16_t  shndx;
} Elf32_Sym;

static Elf32_Sym *elf_symtab   = NULL;
static char      *elf_strtab   = NULL;
static int        elf_symcount = 0;

void init_elf_symbols(void)
{
    elf_symtab   = (Elf32_Sym *)__ksymtab_start;
    elf_strtab   = __kstrtab_start;
    elf_symcount = (__ksymtab_end - __ksymtab_start) / sizeof(Elf32_Sym);

    kprintf("ELF symbols: %d entries\n", elf_symcount);

    // Optional: print some symbols
    for (int i = 0; i < 10 && i < elf_symcount; i++)
    {
        if (elf_symtab[i].value != 0)
        {
            kprintf("  %s = 0x%lx\n", elf_strtab + elf_symtab[i].name, elf_symtab[i].value);
        }
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
