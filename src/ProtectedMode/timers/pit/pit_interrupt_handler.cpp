// pit_interrupt_handler.c
#include "apic.hpp"
#include "assert.h"
#include "atomic.h"
#include "irq.h"
#include "pic_public.h"
#include "pit.h"
#include "pit_internals.h"
#include "pit_interrupt_handler.hpp"
#include "stdio.h"

pit_ih::mode g_pit_mode = pit_ih::mode::wait;

uint32_t		quick_pit_lock = 0;
extern "C" bool quick_pit	   = false;
void			pit_ih::set_quick_path_mode(bool quick_or_not)
{
	// abort_msg("This breaks pit! I have no clue why!\n");
	simple_spin_lock(&quick_pit_lock);
	quick_pit = quick_or_not;
	simple_spin_unlock(&quick_pit_lock);
}

bool pit_ih::get_quick_path_mode()
{
	simple_spin_lock(&quick_pit_lock);
	bool a = quick_pit;
	simple_spin_unlock(&quick_pit_lock);
	return a;
}

void pit_ih::set_mode(enum mode mode)
{
	g_pit_mode = mode;
}

extern "C" void set_mode(PIT_IH_MODE mode)
{

	pit_ih::mode cpp_mode = static_cast<pit_ih::mode>(mode);
	g_pit_mode			  = cpp_mode;
}

void game_loop()
{
	// just import the header with
	// void game_loop();
	// in it.

	// And Move this function someplace else.

	// Since this and keyboard and mouse are IG32
	// Keyboard and mouse interrupt will not happen while the game is running.
	// They will be scheduled to happen after it.
	// Then, this game loop can have a global context of stuff affected by keyboard and mouse.
	// Hmm, first press might be difficult.
	// Release too.
	// But if key is held won't be.
	// Unless i have a clear first press, and clear release at the end of the game loop.
}

extern "C" void pit_interrupt_handler()
{

	switch (g_pit_mode)
	{
	case pit_ih::mode::wait: break;
	case pit_ih::mode::game: game_loop();
	}

	if (pit_write_index != 0 && pit_is_new_timeout)
	{
		pit_write_index -= 1;
	}
	volatile uint32_t *write_address = pit_msg_address[pit_write_index];
	// kprintf("The Write index: %u\n", pit_write_index);
	// kprintf("The Write address: %h\n", write_address);
	// kprintf("The Write value: %h\n", pit_msg_value[pit_write_index]);
	if (write_address != nullptr)
	{
		*pit_msg_address[pit_write_index] = pit_msg_value[pit_write_index];
		pit_is_new_timeout				  = false;
	}

	// kprintf("C pit interrupt handler!\n");
	pit_interrupt_handled = true;
	apic::send_eoi();
	// kprintf("PIT Handler Receiving core: %u\n", apic::get_core_id_fast());
	// PIC_sendEOI(PIT_IRQ);
}
