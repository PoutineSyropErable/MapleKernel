#pragma once
// This file will be used from 32 bit code

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace gdt64_32
{

#define PACKED __attribute__((packed))

typedef struct __attribute__((packed)) segment_selector
{
	uint8_t	 rpl : 2;	 // Requested Privilege Level (0-3)
	bool	 ti : 1;	 // Table Indicator (0=GDT, 1=LDT)
	uint16_t index : 13; // Index into GDT or LDT
} segment_selector_t;

struct PACKED segment_descriptor_32
{
	uint16_t limit_low;
	uint16_t base_low	 = 0;
	uint8_t	 base_middle = 0;

	uint8_t type : 4;
	// Bits 40–43 (Type)
	// Code segment: must be executable
	// Data segment: must be data (SS must be writable)
	// Still USED for validity checking in long mode

	uint8_t s : 1 = 1;
	// Bit 44
	// 1 = code/data descriptor
	// 0 = system descriptor (TSS, LDT)
	// STILL USED

	uint8_t dpl : 2 = 0;
	// Bits 45–46
	// Descriptor Privilege Level (ring 0–3)
	// STILL USED in long mode

	uint8_t p : 1 = 1;
	// Bit 47
	// Present bit
	// MUST be 1 for valid segments
	// STILL USED

	uint8_t limit_high : 4;

	uint8_t avl : 1 = 0;
	// Bit 52
	// Available for software use
	// IGNORED by hardware

	uint8_t long_mode64 : 1;
	// Bit 53
	// 1 = 64-bit code segment (CS only)
	// MUST be 1 for 64-bit code
	// MUST be 0 for data segments

	uint8_t db : 1 = 0;
	// Bit 54
	// Default operand size
	// MUST be 0 when L = 1
	// Ignored otherwise in long mode

	uint8_t g : 1 = 0;
	// Bit 55
	// Granularity
	// IGNORED in long mode (limit is ignored)

	uint8_t base_high = 0;
};

struct PACKED segment_descriptor_64
{
	uint32_t reserved = 0;
	// Bits 0–31: Low base + low limit in legacy formats.
	// In long mode: IGNORED for code/data segments.

	uint8_t reserved2 = 0;
	// Bits 32–39: Base[15:0] continuation in legacy.
	// In long mode: IGNORED.

	uint8_t type : 4;
	// Bits 40–43 (Type)
	// Code segment: must be executable
	// Data segment: must be data (SS must be writable)
	// Still USED for validity checking in long mode

	uint8_t s : 1 = 1;
	// Bit 44
	// 1 = code/data descriptor
	// 0 = system descriptor (TSS, LDT)
	// STILL USED

	uint8_t dpl : 2 = 0;
	// Bits 45–46
	// Descriptor Privilege Level (ring 0–3)
	// STILL USED in long mode

	uint8_t p : 1 = 1;
	// Bit 47
	// Present bit
	// MUST be 1 for valid segments
	// STILL USED

	uint8_t reserved3 : 4 = 0;
	// Bits 48–51
	// Legacy limit high bits
	// IGNORED in long mode

	uint8_t avl : 1 = 0;
	// Bit 52
	// Available for software use
	// IGNORED by hardware

	uint8_t long_mode64 : 1;
	// Bit 53
	// 1 = 64-bit code segment (CS only)
	// MUST be 1 for 64-bit code
	// MUST be 0 for data segments

	uint8_t db : 1 = 0;
	// Bit 54
	// Default operand size
	// MUST be 0 when L = 1
	// Ignored otherwise in long mode

	uint8_t g : 1 = 0;
	// Bit 55
	// Granularity
	// IGNORED in long mode (limit is ignored)

	uint8_t reserved4 = 0;
	// Bits 56–63: Base[31:24] in legacy
	// IGNORED in long mode
};

// Fs and gs base is controlled through msrs.

struct PACKED system_segment_descriptor
{
	/* ---- Low 8 bytes (entry N) ---- */

	uint16_t limit_low;
	// Bits 0–15: Segment limit
	// For TSS: must be >= sizeof(TSS) - 1

	uint16_t base_low;
	// Bits 0–15 of base address

	uint8_t base_middle;
	// Bits 16–23 of base address

	uint8_t type : 4;
	// Bits 40–43
	// 0x9 = Available 64-bit TSS
	// 0xB = Busy 64-bit TSS
	// 0x2 = LDT

	uint8_t s : 1 = 0;
	// Bit 44
	// MUST be 0 (system segment)

	uint8_t dpl : 2;
	// Bits 45–46
	// Privilege level (usually 0)

	uint8_t p : 1;
	// Bit 47
	// Present bit

	uint8_t limit_high : 4;
	// Bits 48–51: High limit bits

	uint8_t avl : 1;
	// Bit 52
	// Available for software use

	uint8_t zero : 2;
	// Bits 53–54
	// MUST be 0 in long mode (L and D/B are invalid here)

	uint8_t g : 1;
	// Bit 55
	// Granularity (usually 0 for TSS)

	uint8_t base_high;
	// Bits 56–63: Base[31:24]

	/* ---- High 8 bytes (entry N+1) ---- */

	uint32_t base_upper;
	// Bits 64–95: Base[63:32]

	uint32_t reserved;
	// Bits 96–127
	// MUST be zero
};

struct __attribute__((packed)) gdtr
{
	uint16_t table_limit;
	uint32_t base_address_low;
	uint32_t base_address_high = 0;
};

struct gdt64_simple
{
	uint64_t				  null_entry = 0; // 0x0
	segment_descriptor_64	  code_segment64; // 0x08
	segment_descriptor_64	  data_segment64; // 0x10
	segment_descriptor_32	  code_segment32; // 0x18
	segment_descriptor_32	  data_segment32; // 0x20
	system_segment_descriptor tss_segment;	  // (0x28, 0x30) -> [0x28, 0x38[
};

} // namespace gdt64_32
