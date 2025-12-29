/* kernel64_linker.ld - Pure Higher-Half Kernel Module Linker */

/* Base virtual address for kernel modules in higher half */
KERNEL_VIRTUAL_BASE = 0xffffffff80000000;  /* Common canonical higher half base */

/* Entry point for the module */
ENTRY(kernel64_start)

/* Define symbols for kernel use */
__kernel_virtual_base = KERNEL_VIRTUAL_BASE;

SECTIONS
{
    /* Start at the higher half virtual address */
    . = KERNEL_VIRTUAL_BASE;
    
    /* Text section - executable code */
    .text ALIGN(0x1000) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE)
    {
        *(.text)
        *(.text.*)
        . = ALIGN(0x1000);
    }
    
    /* Read-only data */
    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
    {
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(0x1000);
    }
    
    /* Kernel data - read/write, initialized */
    .data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        *(.data)
        *(.data.*)
        . = ALIGN(0x1000);
    }
    
    /* BSS - uninitialized data, will be zeroed */
    .bss ALIGN(0x1000) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE)
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
        . = ALIGN(0x1000);
    }
    
    /* Stack for this module */
    .stack ALIGN(0x1000) : AT(ADDR(.stack) - KERNEL_VIRTUAL_BASE)
    {
        __stack_bottom = .;
        . += 0x4000;  /* 16KB stack */
        __stack_top = .;
        . = ALIGN(0x1000);
    }
    
    /* Heap for this module */
    .heap ALIGN(0x1000) : AT(ADDR(.heap) - KERNEL_VIRTUAL_BASE)
    {
        __heap_start = .;
        . += 0x10000;  /* 64KB initial heap */
        __heap_end = .;
        . = ALIGN(0x1000);
    }
    
    /* End of the module */
    . = ALIGN(0x1000);
    __module_end = .;
    
    /* Useful symbols for debugging/management */
    __text_start = ADDR(.text);
    __text_end = ADDR(.text) + SIZEOF(.text);
    __rodata_start = ADDR(.rodata);
    __rodata_end = ADDR(.rodata) + SIZEOF(.rodata);
    __data_start = ADDR(.data);
    __data_end = ADDR(.data) + SIZEOF(.data);
    
    /* Physical addresses (for loading) */
    __load_start = LOADADDR(.text);
    __load_end = LOADADDR(.heap) + SIZEOF(.heap);
    
    /* Total module size */
    __module_size = __module_end - KERNEL_VIRTUAL_BASE;
    __load_size = __load_end - __load_start;
    
    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note.*)
        *(.eh_frame*)
        *(.interp)
        *(.dynamic)
    }
}
