#include "realmode_functions.h"
#include <stdint.h>

/*

Warning, -O0 can add
push ss
pop ds

at the end of the function, which breaks since i have it so ss != ds.
This will make it so there's a weird bug where things don't work. And a tripple fault is caused on long jump to resume32

*/

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
__attribute__((optimize("O1"), section(".text.realmode_functions"))) uint16_t add16_ref(uint16_t a, uint16_t b) {

	return 2 * a + b;
}

/* Reference version (purely for comparison) */
__attribute__((optimize("O1"), section(".text.realmode_functions"))) uint16_t ret_5() {

	return 5;
}

/* Reference version (purely for comparison) */
__attribute__((section(".text.realmode_functions"))) int16_t complex_operation(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {

	return 2 * a + b - c + 3 * d;
}
