#include "realmode_functions.h"
#include <stdint.h>

/* Place this function into the custom section ".text.realmode_functions" */
__attribute__((naked, section(".text.realmode_functions"))) uint16_t add16(uint16_t a, uint16_t b) {
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

/* Reference version (purely for comparison) */
__attribute__((section(".text.realmode_functions"))) uint16_t add16_ref(uint16_t a, uint16_t b) {

	return 2 * a + b;
}

/* Reference version (purely for comparison) */
__attribute__((section(".text.realmode_functions"))) int16_t complex_operation(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {

	return 2 * a + b - c + 3 * d;
}
