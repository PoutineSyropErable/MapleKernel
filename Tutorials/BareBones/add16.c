// add16.c
#include <stdint.h>

__attribute__((naked))
uint16_t
add16(__attribute__((unused)) uint16_t a,
      __attribute__((unused)) uint16_t b) {
	__asm__(
	    ".code16\n"
	    ".intel_syntax noprefix\n"

	    "push bp\n"
	    "mov bp, sp\n"

	    "mov ax, [bp+4]\n"
	    "add ax, [bp+6]\n"

	    "pop bp\n"
	    "ret\n"

	    ".att_syntax\n");
}

uint16_t add16_ref(uint16_t a, uint16_t b) {
	uint16_t result;
	__asm__(".code16\n"
	        ".intel_syntax noprefix\n"
	        "mov ax, %1\n"
	        "add ax, %2\n"
	        "mov %0, ax\n"
	        ".att_syntax\n"
	        : "=r"(result)
	        : "r"(a), "r"(b)
	        : "ax");
	return result;
}
