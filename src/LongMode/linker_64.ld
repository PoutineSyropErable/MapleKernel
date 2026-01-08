/* kernel64_linker.ld - Pure Higher-Half Kernel Module Linker */
/* With proper guard pages between sections */

/* Base virtual address for kernel modules in higher half */
KERNEL_VIRTUAL_BASE = 0xffffffff80000000;

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
        __text_start = .;
        *(.text.entry)
        *(.text)
        *(.text.*)
        __text_end = .;
        . = ALIGN(0x1000);
    }
    
    /* Guard page after text - marked NOLOAD to avoid occupying file space */
    .text_guard (NOLOAD) : AT(ADDR(.text_guard) - KERNEL_VIRTUAL_BASE)
    {
        . = ALIGN(0x1000);
        __text_guard_start = .;
        . += 0x1000;  /* 4KB guard page */
        __text_guard_end = .;
    }
    
    /* Read-only data */
    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
    {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        __rodata_end = .;
        . = ALIGN(0x1000);
    }
    
    /* Guard page after rodata */
    .rodata_guard (NOLOAD) : AT(ADDR(.rodata_guard) - KERNEL_VIRTUAL_BASE)
    {
        . = ALIGN(0x1000);
        __rodata_guard_start = .;
        . += 0x1000;
        __rodata_guard_end = .;
    }
    
    /* Kernel data - read/write, initialized */
    .data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        __data_start = .;
        *(.data)
        *(.data.*)
        __data_end = .;
        . = ALIGN(0x1000);
    }
    
    /* Guard page after data */
    .data_guard (NOLOAD) : AT(ADDR(.data_guard) - KERNEL_VIRTUAL_BASE)
    {
        . = ALIGN(0x1000);
        __data_guard_start = .;
        . += 0x1000;
        __data_guard_end = .;
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
    
    /* Guard page after BSS */
    .bss_guard (NOLOAD) : AT(ADDR(.bss_guard) - KERNEL_VIRTUAL_BASE)
    {
        . = ALIGN(0x1000);
        __bss_guard_start = .;
        . += 0x1000;
        __bss_guard_end = .;
    }
    
    /* Stack for this module */
    .stack ALIGN(0x1000) : AT(ADDR(.stack) - KERNEL_VIRTUAL_BASE)
    {
        __stack_bottom = .;
        . += 0x4000;  /* 16KB stack */
        __stack_top = .;
        . = ALIGN(0x1000);
    }
    
    /* Guard page after stack */
    .stack_guard (NOLOAD) : AT(ADDR(.stack_guard) - KERNEL_VIRTUAL_BASE)
    {
        . = ALIGN(0x1000);
        __stack_guard_start = .;
        . += 0x1000;
        __stack_guard_end = .;
    }
    
    /* Heap for this module */
    .heap ALIGN(0x1000) : AT(ADDR(.heap) - KERNEL_VIRTUAL_BASE)
    {
        __heap_start = .;
        . += 0x10000;  /* 64KB initial heap */
        __heap_end = .;
        . = ALIGN(0x1000);
    }
    
    /* Guard page after heap */
    .heap_guard (NOLOAD) : AT(ADDR(.heap_guard) - KERNEL_VIRTUAL_BASE)
    {
        . = ALIGN(0x1000);
        __heap_guard_start = .;
        . += 0x1000;
        __heap_guard_end = .;
    }
    
    /* End of the module */
    . = ALIGN(0x1000);
    __module_end = .;
    
    /* Physical addresses (for loading) - guard pages excluded since NOLOAD */
    __load_start = LOADADDR(.text);
    __load_end = LOADADDR(.heap) + SIZEOF(.heap);
    
    /* Total module size including guard pages (virtual address space) */
    __module_size = __module_end - KERNEL_VIRTUAL_BASE;
    __load_size = __load_end - __load_start;


    /* Another way to obtain these symbols */
    /* __text_start = ADDR(.text); */
    /* __text_end = ADDR(.text) + SIZEOF(.text); */
    /* __rodata_start = ADDR(.rodata); */
    /* __rodata_end = ADDR(.rodata) + SIZEOF(.rodata); */
    /* __data_start = ADDR(.data); */
    /* __data_end = ADDR(.data) + SIZEOF(.data); */
    
    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note.*)
        *(.eh_frame*)
        *(.interp)
        *(.dynamic)
    }
}
