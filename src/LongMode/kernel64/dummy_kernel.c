// dummy_kernel.c
// Creates a ~4MB ELF object file with separate sections

// Make sure sections are aligned to 4KB
#define ALIGN4K __attribute__((aligned(0x1000)))
// #define SECTION_SIZE (1024 * 1024)
#define SECTION_SIZE (1)

__attribute__((section(".text"))) char dummy_text[SECTION_SIZE] ALIGN4K = {0}; // 1 MB in .text

__attribute__((section(".rodata"))) char dummy_rodata[SECTION_SIZE] ALIGN4K = {0}; // 1 MB in .rodata

__attribute__((section(".data"))) char dummy_data[SECTION_SIZE] ALIGN4K = {0}; // 1 MB in .data

__attribute__((section(".bss"))) char dummy_bss[SECTION_SIZE] ALIGN4K; // 1 MB in .bss (uninitialized)
