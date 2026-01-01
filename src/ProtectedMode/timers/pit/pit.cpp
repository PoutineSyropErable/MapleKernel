#include "assert.h"
#include "intrinsics.h"
#include "irq.h"
#include "pairs.hpp"
#include "pic_public.h"
#include "pit.h"
#include "pit.hpp"
#include "pit_internals.h"
#include "pit_internals.hpp"
#include "stdio.h"
// ===================== Start of CPP stuff
using namespace pit;

inline void send_byte_to_pit(IOPort channel, uint8_t byte)
{
	__outb(static_cast<uint8_t>(channel), byte);
}

inline void send_command(IOPort channel, mode_command_register command)
{
	__outb(static_cast<uint8_t>(channel), static_cast<uint8_t>(command));
}

inline uint8_t read_from_channel(IOPort channel)
{
	return __inb(static_cast<uint8_t>(channel));
}

inline void set_operation_mode(mode_command_register operation_mode)
{
	send_command(pit::IOPort::mode_or_command_register, operation_mode);
}

// ====================
uint16_t read_pit_count(void)
{

	// Disable interrupts
	__cli();

	// al = channel in bits 6 and 7, remaining bits clear
	mode_command_register get_latch_count_command = {//
		.bcd_binary_mode = MCR::Bit_0::binary16,
		.operating_mode	 = MCR::Bit_1_3::single_shot,
		.access_mode	 = MCR::Bit_4_5::latch_count_get_value_cmd,
		.channel		 = MCR::Bit_6_7::channel_0};
	send_command(pit::IOPort::mode_or_command_register, get_latch_count_command);

	uint16_t count = 0;
	count		   = read_from_channel(pit::IOPort::channel_0_data_port); // Low byte
	count |= read_from_channel(pit::IOPort::channel_0_data_port) << 8;	  // High Byte

	return count;
}

/*
This function disable interrupts, after calling it, interrupts must be re-enabled some way.
In particular, reenable it once you're safe and you've already start entering your wait loop
*/
void set_pit_count(uint16_t count)
{
	// Disable interrupts
	__cli();
	send_byte_to_pit(IOPort::channel_0_data_port, count & 0xFF);		  // Low byte
	send_byte_to_pit(IOPort::channel_0_data_port, (count & 0xFF00) >> 8); // High Byte
	// Placing an sti here breaks it. Dont do it!
	// It will race condition.
	// Even if you think it won't.
	// (So little code will execute, surely the count won't be finished yet)
	// It fucking will.
	// My guess is the remainder with low count will fuck it
	// Or just, pure evil cpu microinstruction reordering will break it
	return;
}

// ================= Implementation of usefull functions =====================
/*
This function also sets the current operation modes, in loop, it's not needed.
It causes a repeat.

Precondition: freq_divider_count <= MAX_FREQ_DIVIDER + 1 (65536, or 2^something)
	- If in debug, it will be checked and abbort.
	- If not in debug, undefined behavior.
	- This is an internal function, it shouldn't be called externally anyway.

*/
void pit::send_wait_count_command(uint32_t freq_divider_count)
{

#ifdef DEBUG
	assert(freq_divider_count <= MAX_FREQ_DIVIDER + 1, "Too high a count, undefined behavior!\n");
#endif
	if (freq_divider_count == (MAX_FREQ_DIVIDER + 1))
	{
		freq_divider_count = 0;
	}

	mode_command_register wait_command = {//
		.bcd_binary_mode = MCR::Bit_0::binary16,
		.operating_mode	 = MCR::Bit_1_3::single_shot,
		.access_mode	 = MCR::Bit_4_5::low_then_high_byte,
		.channel		 = MCR::Bit_6_7::channel_0};
	set_operation_mode(wait_command);
	// kprintf("The command byte: %b:8\n", wait_command);

	set_pit_count(freq_divider_count);
}

// This function will disable interrupts. We need something else to re-enable it
// It will setup a frequency rate generator. That sends an interrupt every PIT_FREQ_HZ / freq_divider_count
// Precondition: freq_divider_count <= MAX_FREQ_DIVIDER + 1 (65536, or 2^something)
// 	- If in debug, it will be checked and abbort.
// 	- If not in debug, undefined behavior.
// 	- This is an internal function, it shouldn't be called externally anyway.
void pit::start_loop_wait(uint32_t freq_divider_count)
{
#ifdef DEBUG
	assert(freq_divider_count <= MAX_FREQ_DIVIDER + 1, "Too high a count, undefined behavior!\n");
#endif
	if (freq_divider_count == (MAX_FREQ_DIVIDER + 1))
	{
		freq_divider_count = 0;
	}

	mode_command_register wait_command = {//
		.bcd_binary_mode = MCR::Bit_0::binary16,
		.operating_mode	 = MCR::Bit_1_3::rate_generator,
		.access_mode	 = MCR::Bit_4_5::low_then_high_byte,
		.channel		 = MCR::Bit_6_7::channel_0};
	set_operation_mode(wait_command);
	// kprintf("The command byte: %b:8\n", wait_command);

	set_pit_count(freq_divider_count);
}

// ================= Implementation of  wait =====================
// pit.cpp
extern "C"
{
	// I'mma keep the extern C. It's a nice way to say they are used by C or assembly.
	// And, it can be used to escape namespace mangling
	volatile bool	   pit_interrupt_handled;
	volatile bool	   pit_is_new_timeout;
	volatile uint8_t   pit_write_index = 0;
	volatile uint32_t *pit_msg_address[PIT_MAX_MSG];
	volatile uint32_t  pit_msg_value[PIT_MAX_MSG];
}
struct pit_wait_split
{
	uint32_t full_cycles;
	uint32_t reminder_pit_count;
};
constexpr pit_wait_split compute_pit_wait(float seconds, float max_single_wait, uint32_t max_freq_div)
{
	float f_interrupt_count = seconds / max_single_wait;

	uint32_t full = static_cast<uint32_t>(f_interrupt_count);

	float remainder_seconds = seconds - (full * max_single_wait);

	uint32_t remainder_pit_count = static_cast<uint32_t>(remainder_seconds * (max_freq_div + 1));

	return {full, remainder_pit_count};
}

inline void wait_till_pit_interrupt()
{
	pit_interrupt_handled = false;
	do
	{
		__sti();
		__hlt();
		// The pit interrupt handler will set the global variable pit_interrupt_handled to true.
		// any other interrupt will just cause another loop, where halt will be called.
		// halting the cpu untill the next interrupt.
	} while (!pit_interrupt_handled);
}

/*
This command also changes the mode.
Which is a redundant operation if we are looping single shot interrupt.
But, it works because it changes the interrupt from multishot to single shot.

*/
inline void wait_lte_one_cycle(uint32_t pit_freq_divider)
{
	send_wait_count_command(pit_freq_divider);
	wait_till_pit_interrupt();
}

void pit::wait_pit_count(uint32_t count)
{
	send_wait_count_command(count);
	wait_till_pit_interrupt();
}

int pit::wait(float seconds)
{
	if (seconds <= 0)
	{
		return 1;
	}
	pit_wait_split sp = compute_pit_wait(seconds, max_single_wait, MAX_FREQ_DIVIDER);
	// kprintf("Split result: full cycles: %u, reminder_pit_count : %u\n", sp.full_cycles, sp.reminder_pit_count);
	// This is fine : ^.

	// check we are in single core mode?

	// IRQ_clear_mask(PIT_IRQ);
	if (sp.full_cycles > 0)
	{
		start_loop_wait(MAX_FREQ_DIVIDER + 1);
	}
	for (uint32_t cycle_idx = 0; cycle_idx < sp.full_cycles; cycle_idx++)
	{
		wait_till_pit_interrupt();
		// terminal_writestring("after int\n");
	}

	// The mode change of this command will stop the frequency.
	wait_lte_one_cycle(sp.reminder_pit_count);

	// IRQ_set_mask(PIT_IRQ);
	return 0;
}

int pit::short_timeout_async(float seconds, volatile uint32_t *finished, bool new_timeout)
{
	if (seconds <= 0)
	{
		return 1;
	}
	pit_wait_split sp = compute_pit_wait(seconds, max_single_wait, MAX_FREQ_DIVIDER);

	// check we are in single core mode?
	if (sp.full_cycles > 0)
	{
		return 2;
	}

	if (new_timeout)
	{
		assert(pit_write_index < PIT_MAX_MSG, "Overflowed, too many mesg, out of space\n");
		*finished						 = 0;
		pit_msg_address[pit_write_index] = finished;
		pit_msg_value[pit_write_index]	 = 1;
		pit_is_new_timeout				 = true;
		// :TODO: Note, using this case is dangerous the moment more then one core is active.
		// :RACE Condition:
		pit_write_index += 1;
	}

	// IRQ_clear_mask(PIT_IRQ);
	// ^ This is for pit
	uint32_t pit_freq_divider = sp.reminder_pit_count;
	send_wait_count_command(pit_freq_divider);

	return 0;
}

void pit::short_timeout(uint32_t count)
{
	abort_msg("Not implemented, Just use something else\n");
}
// ===================== End of Cpp Stuff

#ifdef __cplusplus
extern "C"
{
#endif
	// ================ Start of C stuff
	/*
	use pit to wait for x seconds.
	Only works for single core state
	Precondition: seconds > 0
	- Violating this will return 1.
	- Might make it a -DEBUG dependant abort() in the future

	*/
	int wait(float seconds)
	{
		return pit::wait(seconds);
	}

	void wait_test()
	{

		for (uint32_t i = 3; i > 0; i--)
		{
			kprintf("Doing wait test in %u seconds\n", i);
			pit::wait(1);
		}

		for (float t = 0; t < 60; t += 0.5)
		{
			kprintf("t = %f.2\n", t);
			pit::wait(0.5); // attend 0.5 s. Desactive lordinateur pendant l'attente
		}

		kprintf("Out of loop!\n");
	}

// =============== End of C stuff
#ifdef __cplusplus
}
#endif
