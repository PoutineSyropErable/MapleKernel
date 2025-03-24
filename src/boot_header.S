.section .multiboot2
.align 8
.long 0xE85250D6          # magic number
.long 0                  # architecture (0 = i386, 1 = MIPS, we\u2019ll set to 0 for compatibility)
.long header_end - header_start
.long -(0xE85250D6 + 0 + (header_end - header_start))

header_start:

# End tag
.align 8
.short 0                 # type = 0
.short 0                 # flags
.long 8                  # size

header_end:

