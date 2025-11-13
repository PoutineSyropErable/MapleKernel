#include "stdio.h"
#include "string_helper.h"

static inline void nop() {
}

int main(void) {

	// option_num is wrong. Must fix that.
	// Then, to work for the behavior inside the switch statement.
	kprintf("abc%d:27 123%b:5 jkl %p:08 %f.6,", 123, 0b1010, 0x123abc98, 123.576);
	// kprintf("abc%d %f", 123, 225.6);

	return 0;
}
