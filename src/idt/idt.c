#include "assert.h"
#include "idt.h"

#define INTERRUPT_COUNT 256

__attribute__((aligned(0x10))) static idt32_entry_t idt[INTERRUPT_COUNT];

static idtr_t idtr;

// __attribute__((noreturn)) void exception_handler(void);
__attribute__((noreturn)) void exception_handler(void) {
	while (1) {
		__asm__ volatile("cli; hlt"); // Completely hangs the computer
	}
}

void idt_set_descriptor(uint8_t vector, void* isr, enum gate_type32_t gate_type, uint8_t dpl, bool present) {
	assert(is_valid_gate_type32(gate_type), "Gate type is not valid");
	idt32_entry_t* descriptor = &idt[vector];

	descriptor->isr_offset_low = (uint32_t)isr & 0xFFFF;
	descriptor->isr_offset_high = (uint32_t)isr >> 16;

	descriptor->kernel_cs = (segment_selector_t){.rpl = 0, .ti = 0, .index = 2}; // this value can be whatever offset your kernel code selector is in your GDT
	descriptor->dpl = dpl;
	descriptor->present = present;
	descriptor->gate_type = gate_type;

	descriptor->reserved = 0;
	descriptor->bit_44_is_zero = 0;
}
