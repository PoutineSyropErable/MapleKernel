#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "Main.h"
#include "float_helpers.h"
#include "stdio.h"

#include "framebuffer.c"

void kernel_main(uint32_t mb2_info_addr, uint32_t magic, uint32_t is_proper_multiboot_32) {

	kprintf("\n=====start of kernel=====\n");
	kprintf("%c", 'A');
	kprintf("%c", '-');
	kprintf("\n");
	kprintf("%f", 25.0);

	return;
	struct info* mbi = (struct info*)mb2_info_addr;
	struct framebuffer_info_t fb = find_framebuffer(mbi);

	volatile uint32_t* pixels = (volatile uint32_t*)(fb.base_addr_low);

	serial_printf("Framebuffer addr: ", fb.base_addr_low, false);
	serial_printf("Width: ", fb.width, true);
	serial_printf("Height: ", fb.height, true);
	serial_printf("Pitch: ", fb.pitch, true);
	serial_printf("BPP: ", fb.bpp, true);

	// Fill the framebuffer with a simple color pattern
	for (uint32_t y = 0; y < fb.height; y++) {
		for (uint32_t x = 0; x < fb.width; x++) {
			// Pixel offset in the framebuffer
			uint32_t offset = y * (fb.pitch / 4) + x;

			// Write some color: 0xAARRGGBB
			pixels[offset] = 0xFF00FF; // Magenta
		}
	}

	module_main();
}
