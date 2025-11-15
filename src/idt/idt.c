#include "assert.h"
#include "gdt.h"
#include "idt.h"
#include "more_types.h"
#include "stdio.h"

#define IDT_MAX_VECTOR_COUNT 256

__attribute__((aligned(0x10))) static idt32_entry_t idt[IDT_MAX_VECTOR_COUNT];
static idtr_t idtr;

static bool vectors[IDT_MAX_VECTOR_COUNT];
extern void* isr_stub_table[];

// __attribute__((noreturn)) void exception_handler(void);
__attribute__((noreturn)) void exception_handler(void) {
	while (1) {
		__asm__ volatile("cli; hlt"); // Completely hangs the computer
	}
}

extern function_t interrupt_8_handler;  // not a function pointer. It's value is therefor the first few bytes of code
extern function_t interrupt_34_handler; // not a function pointer. It's value is therefor the first few bytes of code
extern function_t interrupt_69_handler; // not a function pointer. It's value is therefor the first few bytes of code

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

void idt_init() {
	idtr.base_address = idt;
	idtr.limit = (uint16_t)sizeof(idt32_entry_t) * IDT_MAX_VECTOR_COUNT - 1;

	for (uint8_t vector = 0; vector < 32; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector], GT32_IG32, 0, true);
		vectors[vector] = true;
	}

	idt_set_descriptor(34, &interrupt_34_handler, GT32_IG32, 0, true);
	vectors[34] = true;

	idt_set_descriptor(69, &interrupt_69_handler, GT32_IG32, 0, true);
	vectors[69] = true;

	idt_set_descriptor(8, &interrupt_8_handler, GT32_IG32, 0, true);
	vectors[8] = true;

	// __asm__ volatile("lidt %0" : : "m"(idtr)); // load the new IDT
	// __asm__ volatile("sti");                   // set the interrupt flag

	__lidt(idtr);
	__sti();

	kprintf("The address of interrupt_31_handler %h\n", isr_stub_table[31]);
	kprintf("The address of interrupt_34_handler %h\n", &interrupt_34_handler);
}
