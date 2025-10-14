/* linker.ld */
OUTPUT_FORMAT("binary")        /* produce a flat binary */
ENTRY(start16)                 /* entry point at 16-bit reset vector */

SECTIONS
{
    /* -----------------------------
       16-bit stage (boot16 + boot16_2)
       ----------------------------- */
    . = 0xFFFFFFF0;            /* reset vector load address */
    .text16 : { *(.text16) }

    /* -----------------------------
       32-bit stage (boot32)
       ----------------------------- */
    . = ALIGN(16);              /* optional alignment */
    .text32 : { *(.text32) }

    /* -----------------------------
       64-bit stage (boot64)
       ----------------------------- */
    . = ALIGN(16);
    .text64 : { *(.text64) }

    /* -----------------------------
       64-bit data
       ----------------------------- */
    . = ALIGN(16);
    .data64 : { *(.data64) }
}

