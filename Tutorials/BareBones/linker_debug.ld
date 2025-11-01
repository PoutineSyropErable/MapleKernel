/* Entry point of the kernel */
ENTRY(boot32_start)

/* Sections layout */
SECTIONS
{


    /* -----------------------------
       16-bit real mode code & data
       ----------------------------- */
    . = 0x7000;              /* start at 28 KiB, safely below 1 MB */


    /* Main text section */
    .multiboot_start BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)         /* multiboot header */
    }

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
    .text16 : ALIGN(16)
    {
        add1616_start = .;
        KEEP(*(.text.add1616))  /* add16_wrapper32.s */
        KEEP(*(.text.add16))     /* add16.c */
    }


	.resume32_low : ALIGN(16)
	{
		add1632_start = .;
		KEEP(*(.text.add1632))   /* 32-bit wrapper code */
		add1632_end = .;
	}


	. =  0x20000;
	.boot32 : ALIGN(16)
    {
        boot32_start = .;
        KEEP(*(.text.boot32))
        KEEP(*(.data.boot32))
        KEEP(*(.bss.boot32))
        boot32_end = .;
    }


    /* -----------------------------
       32-bit protected mode code
       ----------------------------- */
    . = 0x200000;    


    /* Main text section */
    .text BLOCK(4K) : ALIGN(4K)
    {
        /* *(.multiboot)         /* multiboot header */ */
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

