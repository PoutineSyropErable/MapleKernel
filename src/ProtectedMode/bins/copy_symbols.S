/* ./copy_symbols.S - IMPROVED */
.section .ksymtab, "aw", @progbits
.align 4

.global __ksymtab_start
__symbols_c:

/* Include the raw symbol table binary */
.incbin "bins/symbols.bin"

.global __ksymtab_end  
__symbols_c:

