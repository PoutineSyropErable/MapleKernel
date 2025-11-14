#include <stdbool.h>
#include <stdint.h>

#define PACKED __attribute__((packed))

// 32-bit / legacy segment descriptor (code/data)
typedef struct PACKED segment_descriptor {
	uint16_t limit_low;     // bits 0-15 of segment limit
	uint16_t base_low;      // bits 0-15 of base
	uint8_t base_middle;    // bits 16-23 of base
	uint8_t type : 4;       // 40-43
	uint8_t s : 1;          // 44, 1 = code/data, 0 = system
	uint8_t dpl : 2;        // 45-46, privilege level
	uint8_t p : 1;          // 47, present
	uint8_t limit_high : 4; // 48-51
	uint8_t avl : 1;        // 52
	uint8_t l : 1;          // 53, 64-bit code segment
	uint8_t db : 1;         // 54, 0 = 16-bit, 1 = 32-bit
	uint8_t g : 1;          // 55, granularity
	uint8_t base_high;      // 56-63
} segment_descriptor_t;

// :1 says it's a 1 bit field

_Static_assert(sizeof(segment_descriptor_t) == 8, "Segment descriptor must be 8 bytes");

// 64-bit TSS descriptor (long mode) // Takes 2 gdt entry
typedef struct PACKED tss_descriptor_64 {
	uint16_t limit_low;     // bits 0-15 of limit
	uint16_t base_low;      // bits 0-15 of base
	uint8_t base_middle1;   // bits 16-23 of base
	uint8_t type : 4;       // 40-43
	uint8_t s : 1;          // 44, 0 = system
	uint8_t dpl : 2;        // 45-46
	uint8_t p : 1;          // 47
	uint8_t limit_high : 4; // 48-51
	uint8_t avl : 1;        // 52
	uint8_t l : 1;          // 53
	uint8_t db : 1;         // 54
	uint8_t g : 1;          // 55
	uint8_t base_middle2;   // bits 24-31 of base
	uint32_t base_high;     // bits 32-63 of base
	uint32_t reserved;      // must be zero
} tss_descriptor_64_t;

_Static_assert(sizeof(tss_descriptor_64_t) == 16, "64-bit TSS descriptor must be 16 bytes");

// Normal 32-bit segment descriptor reader
static inline segment_descriptor_t segment_descriptor_read(segment_descriptor_t* gdt, uint16_t index) {
	return gdt[index];
}

static inline tss_descriptor_64_t* tss_descriptor_64_fast(segment_descriptor_t* gdt, uint16_t index) {
	// Each TSS takes 2 GDT entries, so index points to the first
	return (tss_descriptor_64_t*)&gdt[index];
}
