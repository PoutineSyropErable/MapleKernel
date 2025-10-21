OUTPUT_FORMAT("elf32-i386")        /* intermediate ELF for symbols/debug */
ENTRY(start16)

SECTIONS
{
    /* 16-bit stage */
    . = 0xFFFFFFF0;
    .text16 : { *(.text16) }
    .data16 : { *(.data16) }

    /* 32-bit stage */
    . = ALIGN(16);
    .bss32 : { *(.bss32) }
    .text32 : { *(.text32) }

    /* 64-bit stage */
    . = ALIGN(16);
    .text64 : { *(.text64) }
    .data64 : { *(.data64) }
}

