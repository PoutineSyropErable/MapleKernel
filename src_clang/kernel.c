#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "Main.h"
#include "float_helpers.h"
#include "stdio.h"

struct info {
	uint32_t total_size;
	uint32_t reserved;
	// tags follow immediately in memory
};

struct tag {
	uint32_t type;
	uint32_t size;
};

// struct framebuffer_info_t {
// 	uintptr_t addr;
// 	uint32_t width;
// 	uint32_t height;
// 	uint32_t pitch;
// 	uint8_t bpp;
// };

struct __attribute__((packed)) framebuffer_info_t {
	uint32_t tag_type;
	uint32_t size;
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
	uint8_t type;
	uint8_t reserved;
	char color_info[];
};

// Multiboot2 tag types
enum {
	TAG_END = 0,
	TAG_FRAMEBUFFER = 8
};

// Helper to get first tag
static struct tag* tag_first(struct info* mbi) {
	return (struct tag*)((uintptr_t)mbi + 8); // skip total_size + reserved
}

// Helper to get next tag (8-byte aligned)
static struct tag* tag_next(struct tag* current) {
	return (struct tag*)(((uintptr_t)current + current->size + 7) & ~7UL);
}

struct framebuffer_info_t find_framebuffer(struct info* mbi) {
	struct framebuffer_info_t fb = {0};

	if (!mbi) {
		// You need to implement your own error handler
		while (1) {
		}
	}

	if (mbi->total_size < 16) { // minimum header + tag
		while (1) {
		}
	}

	struct tag* current = tag_first(mbi);
	uintptr_t end_addr = (uintptr_t)mbi + mbi->total_size;

	while ((uintptr_t)current < end_addr) {
		if (current->type == TAG_END && current->size == 8) {
			break;
		}

		if (current->type == TAG_FRAMEBUFFER) {
			// Framebuffer tag layout (per Multiboot2 spec)
			uint32_t* ptr32 = (uint32_t*)current;
			fb.base_addr_low = *((uintptr_t*)(ptr32 + 2)); // addr at offset 8
			fb.pitch = *(ptr32 + 3);
			fb.width = *(ptr32 + 4);
			fb.height = *(ptr32 + 5);
			fb.bpp = *((uint8_t*)(ptr32 + 6)); // bpp at offset 24

			struct framebuffer_info_t* ret_ptr = (struct framebuffer_info_t*)ptr32;
			struct framebuffer_info_t ret = *ret_ptr;

			return ret;

			// return fb;
		}

		current = tag_next(current);
	}

	// Not found
	while (1) {
	}
}

void kernel_main(uint32_t mb2_info_addr, uint32_t magic, uint32_t is_proper_multiboot_32) {
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
