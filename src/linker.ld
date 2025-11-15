/* Entry point of the kernel */
ENTRY(_start)

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
        . += 48;             /* 48 bytes for arguments + result */
        args16_end = .;
    }

	. += 16;

	.pm32_to_pm16_section : ALIGN(16) 
	{
        pm32_to_pm16_start = .;
		KEEP(*(.text.pm32_to_pm16))   /* 32-bit wrapper code */
		pm32_to_pm16_end = .;
	}

	.saved_sp16 :
	{
		KEEP(*(.bss.saved_sp16))
	}

	.pm16_and_real16_wrappers : ALIGN(16)
	{
        /* call_real16_function = .; */
		KEEP(*(.text.pm16_and_real16_wrappers))   /* 32-bit wrapper code */
		pm16_to_real16_end = .;
	}

	

    /* 16-bit wrapper + add16 function */
    .realmode16_functions : ALIGN(16)
    {
        KEEP(*(.text.realmode_functions))     /* add16.c */
    }


	.resume32 : ALIGN(16)
	{
		KEEP(*(.text.resume32))   /* 32-bit wrapper code */
		resume32_end = .;
	}

	misc32_s1 = .;
	.misc32 : ALIGN(16) 
	{
	misc32_s2 = .;
		*(.data.misc32.*)
		*(.text.misc32.*)
	}



    /* -----------------------------
       32-bit protected mode code
       ----------------------------- */
    . = 0x200000;            /* 2 MiB start for 32-bit kernel */

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


