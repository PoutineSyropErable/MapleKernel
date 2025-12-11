#include "assert.h"
#include "idt.h"
#include "idt_ps2.h"
#include "more_types.h"
#include "pic.h"
#include "pic_ps2.h"
#include "ps2_keyboard_public.h"
#include "ps2_mouse_public.h"

void idt_set_descriptor(uint8_t vector, void *isr, enum gate_type32_t gate_type, uint8_t dpl, bool present);

void bad_interrupt_handler()
{
    abort_msg("Bad interrupt handler!\n");
}

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

void idt_init_ps2(struct idt_init_ps2_fields args)
{
    switch (args.type)
    {
    case ITT_one_keyboard_one_mouse:

	struct idt_fields_keyboard_mouse args_km_value = args.value.info_keyboard_and_mouse;
	uint8_t                          keyboard_port = args_km_value.keyboard_port;
	uint8_t                          mouse_port    = args_km_value.mouse_port;

	// Will need to use them at some point
	enum ps2_device_type keyboard_type = args_km_value.keyboard_type;
	enum ps2_device_type mouse_type    = args_km_value.mouse_type;

	assert(mouse_port == 1 || mouse_port == 2, "Mouse port out of range");
	assert(keyboard_port == 1 || keyboard_port == 2, "keyboard port out of range");
	assert(keyboard_port != mouse_port, "One device per port!\n");

	// function pointers vectors. An if satement could be transformed into a cmovs...
	const uint8_t  keyboard_interrupt_vector  = interrupt_vector_of_port[keyboard_port];
	function_t_ptr keyboard_interrupt_handler = keyboard_interrupt_handlers[keyboard_port];
	idt_set_descriptor(keyboard_interrupt_vector, keyboard_interrupt_handler, GT32_IG32, 0, true);
	vectors[keyboard_interrupt_vector] = true;

	const uint8_t  mouse_interrupt_vector  = interrupt_vector_of_port[mouse_port];
	function_t_ptr mouse_interrupt_handler = mouse_interrupt_handlers[mouse_port];
	idt_set_descriptor(mouse_interrupt_vector, mouse_interrupt_handler, GT32_IG32, 0, true);
	vectors[mouse_interrupt_vector] = true;

	break;
    case ITT_two_keyboard:
	struct idt_fields_2_keyboard args_kk_value  = args.value.info_2_keyboard;
	enum ps2_device_type         keyboard1_type = args_kk_value.keyboard1_type;
	enum ps2_device_type         keyboard2_type = args_kk_value.keyboard2_type;

	const uint8_t keyboard1_interrupt_vector = PS2_PORT1_INTERUPT_VECTOR;
	idt_set_descriptor(keyboard1_interrupt_vector, keyboard_interrupt_handler_port1, GT32_IG32, 0, true);
	vectors[keyboard1_interrupt_vector] = true;

	const uint8_t keyboard2_interrupt_vector = PS2_PORT2_INTERUPT_VECTOR;
	idt_set_descriptor(keyboard2_interrupt_vector, keyboard_interrupt_handler_port2, GT32_IG32, 0, true);
	vectors[keyboard2_interrupt_vector] = true;

	break;
    case ITT_two_mouse:

	struct idt_fields_2_mouse args_mm_value = args.value.info_2_mouse;
	enum ps2_device_type      mouse1_type   = args_mm_value.mouse1_type;
	enum ps2_device_type      mouse2_type   = args_mm_value.mouse2_type;

	const uint8_t mouse1_interrupt_vector = PS2_PORT1_INTERUPT_VECTOR;
	const uint8_t mouse2_interrupt_vector = PS2_PORT2_INTERUPT_VECTOR;

	idt_set_descriptor(mouse1_interrupt_vector, mouse_interrupt_handler_port1, GT32_IG32, 0, true);
	idt_set_descriptor(mouse2_interrupt_vector, mouse_interrupt_handler_port2, GT32_IG32, 0, true);

	vectors[mouse1_interrupt_vector] = true;
	vectors[mouse2_interrupt_vector] = true;
	break;
    case ITT_one_keyboard:

	struct idt_fields_1_keyboard args_k_value                     = args.value.info_1_keyboard;
	enum ps2_device_type         single_keyboard_type             = args_k_value.keyboard_type;
	const uint8_t                single_keyboard_interrupt_vector = PS2_PORT1_INTERUPT_VECTOR;
	idt_set_descriptor(single_keyboard_interrupt_vector, keyboard_interrupt_handler_port1, GT32_IG32, 0, true);
	break;
    case ITT_one_mouse:
	struct idt_fields_1_mouse args_m_value                  = args.value.info_1_mouse;
	enum ps2_device_type      single_mouse_type             = args_m_value.mouse_type;
	const uint8_t             single_mouse_interrupt_vector = PS2_PORT1_INTERUPT_VECTOR;
	idt_set_descriptor(single_mouse_interrupt_vector, mouse_interrupt_handler_port1, GT32_IG32, 0, true);
	break;
    case ITT_no_ps2_device:
	// do nothing
	break;
    }
}
