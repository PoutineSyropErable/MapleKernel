#pragma once
#include "cast.h"
#include "static_assert.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define PACKED __attribute__((packed))

	typedef struct __attribute__((packed)) segment_selector
	{
		uint8_t	 rpl : 2;	 // Requested Privilege Level (0-3)
		bool	 ti : 1;	 // Table Indicator (0=GDT, 1=LDT)
		uint16_t index : 13; // Index into GDT or LDT
	} segment_selector_t;

	STATIC_ASSERT(sizeof(segment_selector_t) == 2, "Segment selector must be 2 bytes");

	// 32-bit / legacy segment descriptor (code/data)
	// :1 says it's a 1 bit field
	// :4 says it's a 4 bit field
	typedef struct PACKED segment_descriptor
	{
		uint16_t limit_low;		 // bits 0-15 of segment limit
		uint16_t base_low;		 // bits 0-15 of base
		uint8_t	 base_middle;	 // bits 16-23 of base
		uint8_t	 type : 4;		 // 40-43
		uint8_t	 s : 1;			 // 44, 1 = code/data, 0 = system
		uint8_t	 dpl : 2;		 // 45-46, privilege level
		uint8_t	 p : 1;			 // 47, present
		uint8_t	 limit_high : 4; // 48-51
		uint8_t	 avl : 1;		 // 52
		uint8_t	 l : 1;			 // 53, 64-bit code segment
		uint8_t	 db : 1;		 // 54, 0 = 16-bit, 1 = 32-bit
		uint8_t	 g : 1;			 // 55, granularity. // g = 0: Byte, G=1 : 4kb chunks (what to calculate limit with)
		uint8_t	 base_high;		 // 56-63
	} segment_descriptor_t;
	STATIC_ASSERT(sizeof(segment_descriptor_t) == 8, "Segment descriptor must be 8 bytes");

	typedef segment_descriptor_t GDT_ENTRY;

	typedef struct __attribute__((packed, aligned(4))) GDTR32
	{
		uint16_t limit;						// size in bytes of the GDT minus 1;
											// number of entries = (limit + 1) / sizeof(segment_selector_t)
		segment_descriptor_t *base_address; // 32-bit base address of the GDT
	} gdtr32_t;

	STATIC_ASSERT(sizeof(gdtr32_t) == 8, "Must be 8 byte due to 4 alignment. (6 real size + 2 pad)");

	static inline uint32_t get_segment_descriptor_base(segment_descriptor_t *desc)
	{
		return ((uint32_t)desc->base_high << 24) | ((uint32_t)desc->base_middle << 16) | desc->base_low;
	}

	static inline void set_segment_descriptor_base(segment_descriptor_t *desc, void *base)
	{
		// Only to be called from 32 bit code

		struct __attribute__((packed)) base_split
		{
			uint16_t low;
			uint8_t	 mid;
			uint8_t	 high;
		};

		struct base_split base_address = BITCAST(struct base_split, base);
		desc->base_low				   = base_address.low;
		desc->base_middle			   = base_address.mid;
		desc->base_high				   = base_address.high;
	}

	static inline void set_segment_descriptor_limit(segment_descriptor_t *desc, uint32_t limit)
	{
		struct __attribute__((packed)) limit_split
		{
			uint16_t low;
			uint16_t high : 4; // Only 4 bits for high limit
		};

		struct limit_split limit_parts = BITCAST(struct limit_split, limit);

		desc->limit_low	 = limit_parts.low;
		desc->limit_high = limit_parts.high;
	}

	static inline uint32_t get_segment_descriptor_limit(segment_descriptor_t *desc)
	{
		return ((uint32_t)desc->limit_high << 16) | ((uint32_t)desc->limit_low);
	}

	static inline gdtr32_t get_gdt_root(void)
	{

		gdtr32_t gdt_root;
		__asm__ volatile("sgdt %0" : "=m"(gdt_root));

		return gdt_root;
	}

	static inline void set_gdt_root(gdtr32_t *gdtr)
	{

		__asm__ volatile("lgdt %0" ::"m"(*gdtr));
	}

#ifdef __cplusplus
}
#endif
