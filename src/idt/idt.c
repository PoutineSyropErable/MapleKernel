#include "assert.h"
#include "gdt.h"
#include "idt.h"
#include "idt_ps2.h"
#include "intrinsics.h"
#include "more_types.h"
#include "pic.h"

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

void bad_interrupt_handler() {
	abort_msg("Bad interrupt handler!\n");
}

extern function_t interrupt_8_handler;  // not a function pointer. It's value is therefor the first few bytes of code
extern function_t interrupt_13_handler; // not a function pointer. It's value is therefor the first few bytes of code
extern function_t interrupt_69_handler; // not a function pointer. It's value is therefor the first few bytes of code

extern function_t keyboard_interrupt_handler_port1; // not a function pointer. It's value is therefor the first few bytes of code
extern function_t keyboard_interrupt_handler_port2; // not a function pointer. It's value is therefor the first few bytes of code
extern function_t mouse_interrupt_handler_port1;    // not a function pointer. It's value is therefor the first few bytes of code
extern function_t mouse_interrupt_handler_port2;    // not a function pointer. It's value is therefor the first few bytes of code

function_t_ptr keyboard_interrupt_handlers[3] = {
    [0] = &bad_interrupt_handler,
    [1] = &keyboard_interrupt_handler_port1,
    [2] = &keyboard_interrupt_handler_port2,
};

function_t_ptr mouse_interrupt_handlers[3] = {
    [0] = &bad_interrupt_handler,
    [1] = &mouse_interrupt_handler_port1,
    [2] = &mouse_interrupt_handler_port2,
};

// typedef struct PACKED {
//     uint32_t eip;
//     uint32_t cs;
//     uint32_t eflags;
// } gcc_intr_frame_t;

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

void idt_init_ps2(struct idt_init_ps2_fields args) {
	switch (args.type) {
	case ITT_one_keyboard_one_mouse:
		kprintf("Setting up the keyboard and mouse stuff, idt entries\n!");

		struct idt_fields_1k_1m args_value = args.value.mk;
		uint8_t keyboard_port = args_value.keyboard_port;
		uint8_t mouse_port = args_value.mouse_port;

		assert(mouse_port == 1 || mouse_port == 2, "Mouse port out of range");
		assert(keyboard_port == 1 || keyboard_port == 2, "keyboard port out of range");
		assert(keyboard_port != mouse_port, "One device per port!\n");

		// if (keyboard_port == 1) {
		// } else if (mouse_port == 2) {
		// }

		const uint8_t keyboard_interrupt_vector = interrupt_vector_of_port[keyboard_port];
		function_t_ptr keyboard_interrupt_handler = keyboard_interrupt_handlers[keyboard_port];
		idt_set_descriptor(keyboard_interrupt_vector, keyboard_interrupt_handler, GT32_IG32, 0, true);
		vectors[keyboard_interrupt_vector] = true;

		const uint8_t mouse_interrupt_vector = interrupt_vector_of_port[mouse_port];
		function_t_ptr mouse_interrupt_handler = mouse_interrupt_handlers[mouse_port];
		idt_set_descriptor(mouse_interrupt_vector, mouse_interrupt_handler, GT32_IG32, 0, true);
		vectors[mouse_interrupt_vector] = true;

		break;
	case ITT_two_keyboard:
		break;
	case ITT_two_mouse:
		break;
	case ITT_one_keyboard:
		break;
	case ITT_one_mouse:
		break;
	case ITT_no_ps2_device:
		// do nothing
		break;
	}
}

void idt_init(struct idt_init_ps2_fields ps2_args) {
	// Or, i could have simply, port_1_type, port_2_type.
	idtr.base_address = idt;
	idtr.limit = (uint16_t)sizeof(idt32_entry_t) * IDT_MAX_VECTOR_COUNT - 1;

	for (uint8_t vector = 0; vector < 32; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector], GT32_IG32, 0, true);
		vectors[vector] = true;
	}

	for (uint8_t vector = 32; vector < 50; vector++) {
		if (vector != 34) {
			idt_set_descriptor(vector, isr_stub_table[vector], GT32_IG32, 0, true);
			vectors[vector] = true;
		}
	}

	idt_init_ps2(ps2_args);
	kprintf("AFter ps2 settup\n");
	// if there are other stuff with runtime args to init, it can go after

	idt_set_descriptor(8, &interrupt_8_handler, GT32_IG32, 0, true);
	vectors[8] = true;

	idt_set_descriptor(13, &interrupt_13_handler, GT32_IG32, 0, true);
	vectors[13] = true;

	// __asm__ volatile("lidt %0" : : "m"(idtr)); // load the new IDT
	// __asm__ volatile("sti");                   // set the interrupt flag

	__lidt(idtr);
	__sti();

	// kprintf("The address of interrupt_31_handler %h\n", isr_stub_table[31]);
	// kprintf("The address of interrupt_34_handler %h\n", &interrupt_34_handler);
}
