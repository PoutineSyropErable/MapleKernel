#include "assert.h"
#include "gdt.h"
#include "idt_master.h"
#include "intrinsics.h"
#include "more_types.h"

// struct idt_init_ps2_fields;
void idt_init_ps2(struct idt_init_ps2_fields args);

__attribute__((aligned(0x10))) static idt32_entry_t idt[IDT_MAX_VECTOR_COUNT];
static idtr_t										idtr;

bool		 vectors[IDT_MAX_VECTOR_COUNT];
extern void *isr_stub_table[];

// __attribute__((noreturn)) void exception_handler(void);
__attribute__((noreturn)) void exception_handler(void)
{
	while (1)
	{
		__asm__ volatile("cli; hlt"); // Completely hangs the computer
	}
}

extern function_t interrupt_8_handler;	// not a function pointer. It's value is therefor the first few bytes of code
extern function_t interrupt_11_handler; // not a function pointer. It's value is therefor the first few bytes of code
extern function_t interrupt_13_handler; // not a function pointer. It's value is therefor the first few bytes of code
extern function_t interrupt_69_handler; // not a function pointer. It's value is therefor the first few bytes of code

// typedef struct PACKED {
//     uint32_t eip;
//     uint32_t cs;
//     uint32_t eflags;
// } gcc_intr_frame_t;

void idt_set_descriptor(uint8_t vector, void *isr, enum gate_type32_t gate_type, uint8_t dpl, bool present)
{
	assert(is_valid_gate_type32(gate_type), "Gate type is not valid");
	idt32_entry_t *descriptor = &idt[vector];

	descriptor->isr_offset_low	= (uint32_t)isr & 0xFFFF;
	descriptor->isr_offset_high = (uint32_t)isr >> 16;

	descriptor->kernel_cs =
		(segment_selector_t){.rpl = 0, .ti = 0, .index = 2}; // this value can be whatever offset your kernel code selector is in your GDT
	descriptor->dpl		  = dpl;
	descriptor->present	  = present;
	descriptor->gate_type = gate_type;

	descriptor->reserved	   = 0;
	descriptor->bit_44_is_zero = 0;
}

void idt_init()
{
	// Or, i could have simply, port_1_type, port_2_type.
	idtr.base_address = idt;
	idtr.limit		  = (uint16_t)sizeof(idt32_entry_t) * IDT_MAX_VECTOR_COUNT - 1;

	for (uint8_t vector = 0; vector < 32; vector++)
	{

		idt_set_descriptor(vector, isr_stub_table[vector], GT32_IG32, 0, true);
		vectors[vector] = true;
	}

	for (uint8_t vector = 32; vector < 50; vector++)
	{
		if (vector != 34)
		{
			idt_set_descriptor(vector, isr_stub_table[vector], GT32_IG32, 0, true);
			vectors[vector] = true;
		}
	}

	// if there are other stuff with runtime args to init, it can go after

	idt_set_descriptor(8, &interrupt_8_handler, GT32_IG32, 0, true);
	vectors[8] = true;

	idt_set_descriptor(11, &interrupt_11_handler, GT32_IG32, 0, true);
	vectors[11] = true;

	idt_set_descriptor(13, &interrupt_13_handler, GT32_IG32, 0, true);
	vectors[13] = true;
}

void idt_finalize()
{
	__lidt(idtr);
	__sti();
}

idt32_entry_t *get_idt_base()
{
	return idt;
}

idt32_entry_t *get_idt_entry(uint8_t entry_index)
{

	uint8_t idt_entry_index_max = idtr.limit / sizeof(idt32_entry_t);
	// if i wanted the count: (idtr.limit + 1)/32.
	// that would be one bigger then entry max.

	assert(entry_index <= idt_entry_index_max, "The entry index: (%u) must be <= the max possible entry: (%u)\n", entry_index,
		idt_entry_index_max);

	return &idt[entry_index];
}

void enable_idt_entry(uint8_t entry_index)
{

	assert(vectors[entry_index], "The entry (%u) must have been enabled once before by a proper setup in idt_init!\n", entry_index);
	idt32_entry_t *entry_ptr = get_idt_entry(entry_index);
	entry_ptr->present		 = true;
}

void disable_idt_entry(uint8_t entry_index)
{

	idt32_entry_t *entry_ptr = get_idt_entry(entry_index);
	entry_ptr->present		 = false;
}
