#include "intrinsics.h"
#include "pit_internals.h"
#include "pit_internals.hpp"

namespace pit
{

namespace
{

static inline void send_byte_to_pit(IOPort channel, uint8_t byte)
{
	__outb(static_cast<uint8_t>(channel), byte);
}

static inline void send_command(IOPort channel, mode_command_register command)
{
	__outb(static_cast<uint8_t>(channel), static_cast<uint8_t>(command));
}

static inline uint8_t read_from_channel(IOPort channel)
{
	return __inb(static_cast<uint8_t>(channel));
}

static inline void set_operation_mode(mode_command_register operation_mode)
{
	send_command(pit::IOPort::mode_or_command_register, operation_mode);
}

static inline void set_pit_count_q(uint16_t count)
{
	__cli();
	send_byte_to_pit(IOPort::channel_0_data_port, count & 0xFF);		  // Low byte
	send_byte_to_pit(IOPort::channel_0_data_port, (count & 0xFF00) >> 8); // High Byte
	return;
}

} // namespace

static inline void send_wait_count_command_q(uint16_t count)
{

#ifdef DEBUG
	assert(freq_divider_count <= MAX_FREQ_DIVIDER + 1, "Too high a count, undefined behavior!\n");
#endif

	mode_command_register wait_command = {//
		.bcd_binary_mode = MCR::Bit_0::binary16,
		.operating_mode	 = MCR::Bit_1_3::single_shot,
		.access_mode	 = MCR::Bit_4_5::low_then_high_byte,
		.channel		 = MCR::Bit_6_7::channel_0};
	set_operation_mode(wait_command);
	// kprintf("The command byte: %b:8\n", wait_command);

	set_pit_count_q(count);
}

inline void wait_till_pit_interrupt_busy()
{
	pit_interrupt_handled = false;
	while (!pit_interrupt_handled)
	{
	}
}

static inline void wait_pit_count_precise(uint32_t pit_freq_divider)
{
	send_wait_count_command_q(pit_freq_divider);
	wait_till_pit_interrupt_busy();
}

} // namespace pit
