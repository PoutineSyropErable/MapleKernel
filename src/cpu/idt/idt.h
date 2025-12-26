#pragma once
#include "gdt.h"
#include "static_assert.h"
#include <stdbool.h>
#include <stdint.h>
#define IDT_MAX_VECTOR_COUNT 256

enum gate_type32_t
{
	GT32_TASK_GATE = 0x5,
	GT32_IG16	   = 0x6,
	GT32_TG16	   = 0x7,
	GT32_IG32	   = 0xe,
	GT32_TG32	   = 0xf
};

static inline bool is_valid_gate_type32(enum gate_type32_t gt)
{
	switch (gt)
	{
	case GT32_TASK_GATE:
	case GT32_IG16:
	case GT32_TG16:
	case GT32_IG32:
	case GT32_TG32: return true;
	default: return false;
	}
}

/*
Offset: 32 bit value, split into two parts.
	Represents the entry point of the interrupt service routine

Selector: A segment seclector, must point to a valid *GDT* entry! (No LDT)

Gate Type: A 4 bit value which define the type of the gate this *Interupt Desciptor* represents. There are 4 valid types
	0b0101 or 0x5: Task Gate, note that in this case, the *Offset* value is unused and should be set to zero
	0b0110 or 0x6: 16 Bit Interrupt Gate
	0b0111 or 0x7: 16 Bit Trap Gate
	0b1110 or 0xe: 32 bit interrupt gate
	0b1111 or 0xf: 32 bit trap gate

DPL: A 2 bit value which define the CPU Priviledge level which are allowed to access this interrupt via the INT instructions. Hardware
interrupts ignore this mechanism. P: Present bit, must be set (1) for the descriptor to be valid.
*/
typedef struct PACKED idt32_entry
{
	uint16_t		   isr_offset_low;
	segment_selector_t kernel_cs;
	uint8_t			   reserved;

	// This is 1byte
	enum gate_type32_t gate_type : 4;
	bool			   bit_44_is_zero : 1;
	uint8_t			   dpl : 2;
	bool			   present : 1;

	uint16_t isr_offset_high;

} idt32_entry_t;

typedef struct __attribute__((packed)) idtr
{
	uint16_t	   limit;		 // the size is the byte count -1, not the number of element
	idt32_entry_t *base_address; // 32 bit adddress
} idtr_t;

STATIC_ASSERT(sizeof(idt32_entry_t) == 8, "IDT entry must be 8 bytes (64 bits) long");
STATIC_ASSERT(sizeof(idtr_t) == 6, "idtr entry must be 6 bytes (48 bits) long");

/*
see `idt32_entry_t` for the other fields
Only two valid gate type
	0b1110 or 0xE: 64 bit Interupt Gate
	0b1111 or 0xF: 64 bit Trap Gate


IST: A 3-bit value which is an offset into the Interrupt Stack Table, which is stored in the Task State Segment.
	If the bits are all set to zero, the Interrupt Stack Table is not used.
*/
typedef struct PACKED idt64_entry
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

STATIC_ASSERT(sizeof(idt64_entry_t) == 16, "IDT64 entry must be 16 bytes (128 bits) long");
// iret  is used in 16 bit interrupt handlers
// iretd is used in 32 bit interrupt handlers
// iretq is used in 64 bit interupts handlers.

static inline void __lidt(idtr_t idt)
{

	__asm__ volatile("lidt %0" : : "m"(idt));
}

void idt_set_descriptor(uint8_t vector, void *isr, enum gate_type32_t gate_type, uint8_t dpl, bool present);
