#pragma once
#include "gdt64.hpp"
#include "static_assert.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace idt64_32
{

typedef struct __attribute__((packed)) segment_selector
{
	uint8_t	 rpl : 2;	 // Requested Privilege Level (0-3)
	bool	 ti : 1;	 // Table Indicator (0=GDT, 1=LDT)
	uint16_t index : 13; // Index into GDT or LDT
} segment_selector_t;

typedef struct __attribute__((packed)) idt64_entry
{
	uint16_t		   offset_low;
	segment_selector_t segment_selector;
	uint8_t			   IST : 3;
	uint8_t			   reserved : 5;

	// This is 1byte
	uint8_t gate_type : 4;
	bool	bit_44_is_zero : 1;
	uint8_t dpl : 2;
	bool	present : 1;

	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved2;

} idt64_entry_t;

typedef struct __attribute__((packed)) idtr
{
	uint16_t limit;			   // the size is the byte count -1, not the number of element
	uint64_t idt_base_address; // 32 bit adddress
} idtr_t;

struct idt64_simple
{
	idt64_entry entries[256];
};

} // namespace idt64_32
