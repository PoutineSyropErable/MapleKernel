#include <stdint.h>

__attribute__((naked))
uint16_t
add16(uint16_t a, uint16_t b) {
	__asm__(
	    ".code16\n" // emit 16-bit instructions
	    ".intel_syntax noprefix\n"

	    "push bp\n"
	    "mov bp, sp\n"

	    "mov ax, [bp+4]\n" // first argument (16-bit)
	    "add ax, [bp+6]\n" // second argument (16-bit)

	    "pop bp\n"
	    "ret\n"

	    ".att_syntax\n");
}
