.section .multiboot2
.align 8
.long 0xE85250D6            # Magic number
.long 0                     # Architecture (0 = i386)
.long header_end - header_start
.long -(0xE85250D6 + 0 + (header_end - header_start)) # Checksum

header_start:

# End tag
.align 8
.short 0                    # Type = 0 (end tag)
.short 0                    # Flags
.long 8                     # Size

header_end:

