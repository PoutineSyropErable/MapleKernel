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
        *(.text.entry)
        *(.text)
        *(.text.*)
        *(.ltext)
        *(.ltext.*)
        . = ALIGN(0x1000);
    }

	/* Text guard - uses guard.asm */
    .text_guard ALIGN(0x1000) : AT(ADDR(.text_guard) - KERNEL_VIRTUAL_BASE)
    {
        __text_guard_start = .;
        *(.guard)  /* References guard.asm */
        __text_guard_end = .;
    }
    
    /* Read-only data */
    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
    {
        *(.rodata)
        *(.rodata.*)
        *(.lrodata)
        *(.lrodata.*)
        . = ALIGN(0x1000);
    }


    .rodata_guard ALIGN(0x1000) : AT(ADDR(.rodata_guard) - KERNEL_VIRTUAL_BASE)
    {
        __rodata_guard_start = .;
        *(.guard)  /* References guard.asm */
        __rodata_guard_end = .;
    }
    
    /* Kernel data - read/write, initialized */
    .data ALIGN(0x1000) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        *(.data)
        *(.data.*)
        *(.ldata)
        *(.ldata.*)
        . = ALIGN(0x1000);
    }


    .data_guard ALIGN(0x1000) : AT(ADDR(.data_guard) - KERNEL_VIRTUAL_BASE)
    {
        __data_guard_start = .;
        *(.guard)  /* References guard.asm */
        __data_guard_end = .;
    }
    
    /* BSS - uninitialized data, will be zeroed */
    .bss ALIGN(0x1000) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE)
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.lbss)
        *(.lbss.*)
        *(COMMON)
        __bss_end = .;
        . = ALIGN(0x1000);
    }


    .bss_guard ALIGN(0x1000) : AT(ADDR(.bss_guard) - KERNEL_VIRTUAL_BASE)
    {
        __bss_guard_start = .;
        *(.guard)  /* References guard.asm */
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


    .stack_guard ALIGN(0x1000) : AT(ADDR(.stack_guard) - KERNEL_VIRTUAL_BASE)
    {
        __stack_guard_start = .;
        *(.guard)  /* References guard.asm */
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


    .heap_guard ALIGN(0x1000) : AT(ADDR(.heap_guard) - KERNEL_VIRTUAL_BASE)
    {
        __heap_guard_start = .;
        *(.guard)  /* References guard.asm */
        __heap_guard_end = .;
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

