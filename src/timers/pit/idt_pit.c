#include "idt.h"
#include "idt_public.h"
#include "more_types.h"
#include "pic_pit.h"

extern function_t pit_interrupt_handler_asm; // not a function pointer. It's value is therefor the first few bytes of code

void idt_init_pit()
{
	uint8_t pit_interrupt_vector = PIT_INTERUPT_VECTOR;
	idt_set_descriptor(pit_interrupt_vector, pit_interrupt_handler_asm, GT32_IG32, 0, true);
}
