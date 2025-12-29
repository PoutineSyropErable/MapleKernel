ENTRY(kernel_main)

SECTIONS {
    . = 2M;

    .multiboot2 : {
        KEEP(*(.multiboot2))
    }

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
    }
}

