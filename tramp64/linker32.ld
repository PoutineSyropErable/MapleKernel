ENTRY(_start)

SECTIONS
{
    /* Start at 1 MiB */
    . = 0x100000;

    /* 32-bit kernel text */
    .text : {
        *(.text)
        KEEP(*(.text.trampoline))   /* 64-bit trampoline stub */
    }

    /* 32-bit data */
    .data : {
        *(.data)
    }

    /* 32-bit bss */
    .bss : {
        *(.bss)
        *(COMMON)
    }

    /* End of 32-bit kernel */
    __kernel32_end = .;
}

