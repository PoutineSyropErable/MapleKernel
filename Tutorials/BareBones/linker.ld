/* Entry point of the kernel */
ENTRY(_start)

/* Sections layout */
SECTIONS
{




    /* -----------------------------
       32-bit protected mode code
       ----------------------------- */
    . = 0x200000;            /* 2 MiB start for 32-bit kernel */

    /* Main text section */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)         /* multiboot header */
        *(.text)              /* 32-bit kernel code */
    }

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Initialized data */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Uninitialized data + main stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* Catch-all for any other sections */
}

