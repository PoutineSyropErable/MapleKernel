ENTRY(kernel_main_64)

SECTIONS
{
    /* High-half virtual address */
    . = 0xFFFFFFFF80000000;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
    }

    __kernel64_end = .;
}

