#include "stdio.h"
#include "string_helper.h"

static inline void nop() {
}

uint8_t binary_size(uint32_t binary_number) {
	if (binary_number == 0)
		return 1;

	unsigned bits = 0;
	while (binary_number > 0) {
		bits++;
		binary_number >>= 1;
	}
	return bits;
}

int main(void) {

	// option_num is wrong. Must fix that.
	// Then, to work for the behavior inside the switch statement.
	// kprintf("abc%d:27 123%b:5 jkl %p:08 %f.6,", 123, 0b1010, 0x123abc98, 123.576);
	// kprintf("abc%d %f", 123, 225.6);

	uint8_t size[64];
	for (int i = 0; i < 64; i++) {
		size[i] = binary_size(i);
	}

	nop();

	return 0;
}
