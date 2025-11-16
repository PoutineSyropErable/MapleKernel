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
// this function won't work in O0, and the optimize("O1") doesn't save it. So don't do functions like that
// In gets compiled with two headers, and two prelude
/*
00000000 <add16>:
   0:	55                   	push   %bp
   1:	89 e5                	mov    %sp,%bp

   3:	55                   	push   %bp
   4:	89 e5                	mov    %sp,%bp

   6:	8b 46 04             	mov    0x4(%bp),%ax
   9:	03 46 06             	add    0x6(%bp),%ax

   c:	5d                   	pop    %bp
   d:	c3                   	ret

   e:	5d                   	pop    %bp
   f:	c3                   	ret




*/
#ifdef BAD_EXAMPLE
__attribute__((naked, section(".text.realmode_functions"), optimize("O1"))) uint16_t add16(uint16_t a, uint16_t b) {
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
#endif

/* Reference version (purely for comparison) */
/* clangd fails to recognize the O1*/
__attribute__((optimize("O1"), section(".text.realmode_functions"))) uint16_t add16_ref(uint16_t a, uint16_t b) {

	return a + b;
}

__attribute__((optimize("O1"), section(".text.realmode_functions"))) uint16_t ret_5() {

	return 5;
}

/* Reference version (purely for comparison) */
__attribute__((section(".text.realmode_functions"))) int16_t complex_operation(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {

	return 2 * a + b - c + 3 * d;
}
