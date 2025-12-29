#include <stdint.h>

extern void jump_to_64bit(void);

void kernel_main_32() {
	// do 32-bit stuff

	// Jump to 64-bit kernel (weak symbol allows link without it)
	jump_to_64bit();

	while (1)
		;
}
