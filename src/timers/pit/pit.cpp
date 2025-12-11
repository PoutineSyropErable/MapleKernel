#include "intrinsics.h"
#include "pairs.hpp"
#include "pit.h"
#include "pit.hpp"
// ===================== Start of CPP stuff
using namespace pit;

constexpr void send_byte_to_pit(IOPort channel, uint8_t byte)
{
    __outb(static_cast<uint8_t>(channel), byte);
}

constexpr void send_command(IOPort channel, mode_command_register command)
{
    __outb(static_cast<uint8_t>(channel), static_cast<uint8_t>(command));
}

constexpr uint8_t read_from_channel(IOPort channel)
{
    return __inb(static_cast<uint8_t>(channel));
}

constexpr void set_operation_mode(mode_command_register operation_mode)
{
    send_command(pit::IOPort::mode_or_command_register, operation_mode);
}

// ====================
unsigned read_pit_count(void)
{

    // Disable interrupts
    __cli();

    // al = channel in bits 6 and 7, remaining bits clear
    mode_command_register get_latch_count_command = {//
        .bcd_binary_mode = MCR::Bit_0::binary16,
        .operating_mode  = MCR::Bit_1_3::single_shot,
        .access_mode     = MCR::Bit_4_5::latch_count_get_value_cmd,
        .channel         = MCR::Bit_6_7::channel_0};
    send_command(pit::IOPort::mode_or_command_register, get_latch_count_command);

    uint16_t count = 0;
    count          = read_from_channel(pit::IOPort::channel_0_data_port); // Low byte
    count |= read_from_channel(pit::IOPort::channel_0_data_port) << 8;    // High Byte

    return count;
}

void set_pit_count(uint16_t count)
{
    // Disable interrupts
    __cli();

    // Set low byte
    __outb(0x40, count & 0xFF);          // Low byte
    __outb(0x40, (count & 0xFF00) >> 8); // High byte
    return;
}

// ================= Implementation of usefull functions =====================
int pit::wait(float seconds)
{
    for (uint32_t i = 0; i < seconds * 1000 * 1000 * 25; i++)
    {
	__nop();
    }
    return 0;
}
// ===================== End of Cpp Stuff

#ifdef __cplusplus
extern "C"
{
#endif
    // ================ Start of C stuff
    int wait(float seconds)
    {
	return pit::wait(seconds);
    }

// =============== End of C stuff
#ifdef __cplusplus
}
#endif
