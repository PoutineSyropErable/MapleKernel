#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __attribute__((packed)) framebuffer_info_t {
	uint32_t tag_type;
	uint32_t size;
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint8_t bit_per_pixel;
	uint8_t type;
	uint8_t reserved;
	char color_info[];
};

#ifdef __cplusplus
}
#endif
