.section .ksymtab, "aw", @progbits
.align 4

.global __ksymtab_start
__ksymtab_start:

/* Include the raw symbol table binary */
.incbin "build/symtab_raw.bin"

.global __ksymtab_end  
__ksymtab_end:

.section .kstrtab, "aw", @progbits
.align 4

.global __kstrtab_start
__kstrtab_start:

/* Include the raw string table binary */
.incbin "build/strtab_raw.bin"

.global __kstrtab_end
__kstrtab_end:

