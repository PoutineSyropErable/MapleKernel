// pit_interrupt_handler.c
#include "irq.h"
#include "pic_public.h"
#include "pit.h"
#include "pit_internals.h"
#include "stdio.h"

void pit_interrupt_handler()
{

	// kprintf("C pit interrupt handler!\n");
	pit_interrupt_handled = true;
	PIC_sendEOI(PIT_IRQ);
}
