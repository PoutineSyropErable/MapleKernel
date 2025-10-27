/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)


/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* It used to be universally recommended to use 1M as a start offset,
	   as it was effectively guaranteed to be available under BIOS systems.
	   However, UEFI has made things more complicated, and experimental data
	   strongly suggests that 2M is a safer place to load. In 2016, a new
	   feature was introduced to the multiboot2 spec to inform bootloaders
	   that a kernel can be loaded anywhere within a range of addresses and
	   will be able to relocate itself to run from such a loader-selected
	   address, in order to give the loader freedom in selecting a span of
	   memory which is verified to be available by the firmware, in order to
	   work around this issue. This does not use that feature, so 2M was
	   chosen as a safer option than the traditional 1M. */

	/* 16-bit stack for wrapper / add16 code */
	.stack16 (NOLOAD) : ALIGN(16)
	{
		stack16_start = .;
		. += 16*1024;      /* 16 KiB */
		stack16_end = .;
	}

	/* Optional: argument storage for 16-bit wrapper */
	.args16 (NOLOAD) : ALIGN(4)
	{
		args16_start = .;
		. += 16;           /* enough to store arguments + result */
		args16_end = .;
	}


	. = 2M;

	/* First put the multiboot header, as it is required to be put very early
	   in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
		*(.text.add1616)  /* 16-bit wrapper */
		*(.text.add1632)  /* 32-bit wrapper */
	}
	/* Allocate a block of 4 kilo byte total. It will be start alligned to 4k, so X000000 (leftmost/least significant)
	 If it doesn't fit, it will be take multiple blocks, going by n*4K where n £ |N 
	 * is for wildcard, it's so that the .text segment of every different file is present. (kernel.o, boot.o) */
	

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
		/* we didn't define .rodata, but kernel.o and boot.o might have them as section in the elf file, so we need to add it */
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		 /* .bss is also present in boot.s */
	}


	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}
