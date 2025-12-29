#include "multiboot.h"
#include "stdio.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Pack the structs to match ELF layout
#pragma pack(push, 1)

typedef struct
{
	uint32_t type;	 // p_type (1 = PT_LOAD)
	uint32_t flags;	 // p_flags
	uint64_t offset; // p_offset
	uint64_t vaddr;	 // p_vaddr
	uint64_t paddr;	 // p_paddr
	uint64_t filesz; // p_filesz
	uint64_t memsz;	 // p_memsz
	uint64_t align;	 // p_align
} elf64_phdr_64bit;

typedef struct
{
	uint32_t name_offset; // st_name
	uint8_t	 info;		  // st_info
	uint8_t	 other;		  // st_other
	uint16_t section_idx; // st_shndx
	uint32_t value_low;	  // st_value (low 32 bits)
	uint32_t value_high;  // st_value (high 32 bits)
	uint32_t size_low;	  // st_size (low 32 bits)
	uint32_t size_high;	  // st_size (high 32 bits)
} elf64_sym_32bit;

typedef struct
{
	uint32_t name_offset; // sh_name
	uint32_t type;		  // sh_type
	uint64_t flags;		  // sh_flags (split later)
	uint64_t addr;		  // sh_addr (split later)
	uint64_t offset;	  // sh_offset (split later)
	uint64_t size;		  // sh_size (split later)
	uint32_t link;		  // sh_link
	uint32_t info;		  // sh_info
	uint64_t addralign;	  // sh_addralign (split later)
	uint64_t entsize;	  // sh_entsize (split later)
} elf64_shdr_64bit;

typedef struct
{
	uint8_t	 ident[16]; // e_ident
	uint16_t type;		// e_type
	uint16_t machine;	// e_machine
	uint32_t version;	// e_version
	uint64_t entry;		// e_entry (split later)
	uint64_t phoff;		// e_phoff (split later)
	uint64_t shoff;		// e_shoff (split later)
	uint32_t flags;		// e_flags
	uint16_t ehsize;	// e_ehsize
	uint16_t phentsize; // e_phentsize
	uint16_t phnum;		// e_phnum
	uint16_t shentsize; // e_shentsize
	uint16_t shnum;		// e_shnum
	uint16_t shstrndx;	// e_shstrndx
} elf64_ehdr_64bit;

#pragma pack(pop)

// Print 64-bit value as hex
static void print_u64_hex(uint64_t val)
{
	uint32_t high = (uint32_t)(val >> 32);
	uint32_t low  = (uint32_t)(val & 0xFFFFFFFF);

	if (high == 0)
	{
		kprintf("%h", low);
	}
	else
	{
		kprintf("%h%h", high, low);
	}
}

// Quick calculation (usually works for kernels):
struct entry_point_c get_entry_point_physical_simple(uint32_t module_phys_addr)
{
	uint8_t			 *mod  = (uint8_t *)module_phys_addr;
	elf64_ehdr_64bit *ehdr = (elf64_ehdr_64bit *)mod;

	kprintf("Module physical: 0x%08x\n", module_phys_addr);
	kprintf("ELF header size: %u bytes\n", sizeof(elf64_ehdr_64bit));

	// Read program header offset (64-bit value)
	uint64_t phoff = ehdr->phoff; // This should be 0x40 for you

	kprintf("Program header offset from ELF: 0x");
	print_u64_hex(phoff);
	kprintf("\n");

	struct entry_point_c ret = {.entry_physical = 0, .entry_virtual = 0, .size = 0};

	// Actually, you need to look at the FIRST program header
	if (phoff == 0 || ehdr->phnum == 0)
	{
		kprintf("No program headers!\n");
		return ret;
	}

	// Program headers start here
	uint8_t *phdr_start = mod + (uint32_t)phoff;

	// The FIRST LOAD segment's offset tells us where code starts
	for (uint16_t i = 0; i < ehdr->phnum; i++)
	{
		// Assuming you have an elf64_phdr struct
		elf64_phdr_64bit *phdr = (elf64_phdr_64bit *)(phdr_start + i * sizeof(elf64_phdr_64bit));

		if (phdr->type == 1)
		{ // PT_LOAD = 1
			kprintf("Found LOAD segment %u:\n", i);
			kprintf("  Type: %u (LOAD)\n", phdr->type);
			kprintf("  Offset in file: 0x");
			print_u64_hex(phdr->offset);
			kprintf("\n");
			kprintf("  Virtual address: 0x");
			print_u64_hex(phdr->vaddr);
			kprintf("\n");
			kprintf("  Physical address: 0x");
			print_u64_hex(phdr->paddr);
			kprintf("\n");
			kprintf("  File size: 0x");
			print_u64_hex(phdr->filesz);
			kprintf("\n");

			// Usually the first LOAD segment contains the entry point
			// Entry point physical = module base + segment offset
			uint32_t entry_physical = module_phys_addr + (uint32_t)phdr->offset;
			kprintf("  -> Entry point PHYSICAL approx: 0x%08x\n", entry_physical);

			ret.entry_physical = entry_physical;
			ret.entry_virtual  = phdr->vaddr;
			ret.size		   = phdr->filesz;
			return ret;
		}
	}

	return ret;
}

// Main function using structs
void print_all_symbols_32bit(uint32_t module_phys_addr)
{
	uint8_t			 *mod  = (uint8_t *)module_phys_addr;
	elf64_ehdr_64bit *ehdr = (elf64_ehdr_64bit *)mod;

	kprintf("=== Parsing 64-bit ELF Module at 0x%08x ===\n", module_phys_addr);

	// 1. Check ELF magic
	if (ehdr->ident[0] != 0x7F || ehdr->ident[1] != 'E' || ehdr->ident[2] != 'L' || ehdr->ident[3] != 'F')
	{
		kprintf("ERROR: Not an ELF file\n");
		return;
	}

	// 2. Check it's 64-bit
	if (ehdr->ident[4] != 2)
	{ // EI_CLASS = 2 for 64-bit
		kprintf("ERROR: Not a 64-bit ELF (class=%d)\n", ehdr->ident[4]);
		return;
	}

	kprintf("ELF Header:\n");
	kprintf("  Type: %u, Machine: %u\n", ehdr->type, ehdr->machine);
	kprintf("  Entry point: ");
	print_u64_hex(ehdr->entry);
	kprintf("\n");
	kprintf("  Section headers: %u entries at offset ", ehdr->shnum);
	print_u64_hex(ehdr->shoff);
	kprintf("\n");

	// 3. Calculate section headers address (handle >4GB)
	uint64_t shoff = ehdr->shoff;
	uint32_t shdr_addr_32bit;

	if (shoff > 0xFFFFFFFF)
	{
		kprintf("WARNING: Section header offset >4GB, may be truncated\n");
		shdr_addr_32bit = module_phys_addr + (uint32_t)shoff;
	}
	else
	{
		shdr_addr_32bit = module_phys_addr + (uint32_t)shoff;
	}

	// 4. Find string table for section names
	elf64_shdr_64bit *shstrtab_shdr = (elf64_shdr_64bit *)(shdr_addr_32bit + ehdr->shstrndx * sizeof(elf64_shdr_64bit));

	char *shstrtab = (char *)(module_phys_addr + (uint32_t)shstrtab_shdr->offset);

	// 5. Find .symtab and .strtab sections
	elf64_shdr_64bit *symtab_shdr = NULL;
	elf64_shdr_64bit *strtab_shdr = NULL;

	for (uint16_t i = 0; i < ehdr->shnum; i++)
	{
		elf64_shdr_64bit *shdr = (elf64_shdr_64bit *)(shdr_addr_32bit + i * sizeof(elf64_shdr_64bit));

		char *section_name = shstrtab + shdr->name_offset;

		if (shdr->type == 2)
		{ // SHT_SYMTAB
			symtab_shdr = shdr;
			kprintf("Found .symtab at ");
			print_u64_hex(shdr->offset);
			kprintf(" (size: ");
			print_u64_hex(shdr->size);
			kprintf(")\n");
		}
		else if (shdr->type == 3 && strcmp(section_name, ".strtab") == 0)
		{ // SHT_STRTAB
			strtab_shdr = shdr;
			kprintf("Found .strtab at ");
			print_u64_hex(shdr->offset);
			kprintf("\n");
		}
	}

	if (!symtab_shdr || !strtab_shdr)
	{
		kprintf("ERROR: Symbol table or string table not found\n");
		return;
	}

	// 6. Parse and print symbols
	uint64_t symtab_offset = symtab_shdr->offset;
	uint64_t strtab_offset = strtab_shdr->offset;

	if (symtab_offset > 0xFFFFFFFF || strtab_offset > 0xFFFFFFFF)
	{
		kprintf("ERROR: Symbol/string table offset >4GB, cannot access from 32-bit\n");
		return;
	}

	elf64_sym_32bit *symtab = (elf64_sym_32bit *)(module_phys_addr + (uint32_t)symtab_offset);
	char			*strtab = (char *)(module_phys_addr + (uint32_t)strtab_offset);

	uint64_t num_symbols = symtab_shdr->size / sizeof(elf64_sym_32bit);

	kprintf("\n=== Symbols (%u total) ===\n", (uint32_t)num_symbols);
	kprintf("Idx  Value               Size                Type Shndx Name\n");
	kprintf("------------------------------------------------------------------------\n");

	for (uint64_t i = 0; i < num_symbols; i++)
	{
		elf64_sym_32bit *sym = &symtab[i];

		// Skip empty names
		if (sym->name_offset == 0)
			continue;

		char *name = strtab + sym->name_offset;

		// Reconstruct 64-bit values
		uint64_t value = ((uint64_t)sym->value_high << 32) | sym->value_low;
		uint64_t size  = ((uint64_t)sym->size_high << 32) | sym->size_low;

		// Get symbol type (lower 4 bits of info)
		uint8_t type	  = sym->info & 0x0F;
		char	type_char = '?';
		if (type == 0)
			type_char = ' '; // STT_NOTYPE
		else if (type == 1)
			type_char = 'O'; // STT_OBJECT
		else if (type == 2)
			type_char = 'F'; // STT_FUNC
		else if (type == 3)
			type_char = 'S'; // STT_SECTION
		else if (type == 4)
			type_char = 'F'; // STT_FILE

		// Get binding (upper 4 bits of info)
		uint8_t bind	  = (sym->info >> 4) & 0x0F;
		char	bind_char = ' ';
		if (bind == 1)
			bind_char = 'L'; // STB_LOCAL
		else if (bind == 2)
			bind_char = 'G'; // STB_GLOBAL
		else if (bind == 3)
			bind_char = 'W'; // STB_WEAK

		// Skip section index 0 (undefined)
		if (sym->section_idx == 0)
			continue;

		kprintf("%3u  ", (uint32_t)i);
		print_u64_hex(value);
		kprintf("  ");
		print_u64_hex(size);
		kprintf("  %c%c  %4u  %s\n", bind_char, type_char, sym->section_idx, name);
	}

	kprintf("\n=== Looking for entry points ===\n");

	// Look for common entry point names
	const char *entry_patterns[] = {"main", "kmain", "_start", "start", "init", "entry", "module_init", "kernel_main", NULL};

	for (uint64_t i = 0; i < num_symbols; i++)
	{
		elf64_sym_32bit *sym = &symtab[i];
		if (sym->name_offset == 0)
			continue;

		char   *name = strtab + sym->name_offset;
		uint8_t type = sym->info & 0x0F;
		uint8_t bind = (sym->info >> 4) & 0x0F;

		// Look for functions (STT_FUNC = 2) with global binding
		if (type == 2 && bind == 2)
		{ // Function & Global
			// Check against entry patterns
			for (int p = 0; entry_patterns[p] != NULL; p++)
			{
				if ((name, entry_patterns[p]) != NULL)
				{
					uint64_t value = ((uint64_t)sym->value_high << 32) | sym->value_low;
					kprintf("POTENTIAL ENTRY: %s at ", name);
					print_u64_hex(value);
					kprintf(" (size: ");
					print_u64_hex(((uint64_t)sym->size_high << 32) | sym->size_low);
					kprintf(")\n");
					break;
				}
			}
		}
	}
}
