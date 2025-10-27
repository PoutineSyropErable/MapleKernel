/* Entry point of the kernel */
ENTRY(_start)

/* Sections layout */
SECTIONS
{
    /* -----------------------------
       16-bit real mode code & data
       ----------------------------- */
    . = 0x7000;              /* start at 28 KiB, safely below 1 MB */

    /* 16-bit stack */
    .stack16 (NOLOAD) : ALIGN(16)
    {
        stack16_start = .;
        . += 16*1024;        /* 16 KiB stack */
        stack16_end = .;
    }

	. += 16;

    /* Arguments / result buffer for 16-bit wrapper */
    .args16 (NOLOAD) : ALIGN(4)
    {
        args16_start = .;
        . += 16;             /* 16 bytes for arguments + result */
        args16_end = .;
    }

	. += 16;

    /* 16-bit wrapper + add16 function */
    .text16 (NOLOAD) : ALIGN(16)
    {
        add1616_start = .;
        KEEP(*(.text.add1616))  /* 16-bit wrapper */
        KEEP(*(.text.add16))     /* actual add16 function */
    }

    /* -----------------------------
       32-bit protected mode code
       ----------------------------- */
    . = 0x200000;            /* 2 MiB start for 32-bit kernel */

    /* Main text section */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)         /* multiboot header */
        *(.text)              /* 32-bit kernel code */
        *(.text.add1632)      /* 32-bit wrapper */
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

