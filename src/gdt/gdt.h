#include <stdbool.h>
#include <stdint.h>

/*
The TSS segment selectors points to an entry in the GDT.
That entry has a base address field (separated in parts (3/4)), which points to the TSS
The TSS has information about the different stack for different priviledge level.
If a priviledge level change is done, then the stack is read from it
*/

#define PACKED __attribute__((packed))

typedef struct __attribute__((packed)) segment_selector {
	uint8_t rpl : 2;     // Requested Privilege Level (0-3)
	bool ti : 1;         // Table Indicator (0=GDT, 1=LDT)
	uint16_t index : 13; // Index into GDT or LDT
} segment_selector_t;

_Static_assert(sizeof(segment_selector_t) == 2, "Segment selector must be 2 bytes");

// 32-bit / legacy segment descriptor (code/data)
// :1 says it's a 1 bit field
// :4 says it's a 4 bit field
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
_Static_assert(sizeof(segment_descriptor_t) == 8, "Segment descriptor must be 8 bytes");

struct __attribute__((packed)) GDTR32 {
	uint16_t gdt_size;                // the size is the byte count -1, not the number of element
	segment_selector_t* base_address; // 32 bit adddress
};

static inline uint32_t get_segment_descriptor_base(segment_descriptor_t* desc) {
	return ((uint32_t)desc->base_high << 24) |
	       ((uint32_t)desc->base_middle << 16) |
	       desc->base_low;
}
