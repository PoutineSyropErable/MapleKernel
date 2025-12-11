#pragma once
#include <stddef.h>
#include <stdint.h>

namespace pit
{

constexpr float PIT_FREQ_MHZ = 1.193182;
constexpr uint32_t PIT_FREQ_HZ = PIT_FREQ_MHZ * 1'000'000;

constexpr uint16_t MAX_FREQ_DIVIDER = 65535; // maybe 0 is mapped to 65536

constexpr uint8_t PIT_IRQ_CHANEL0 = 0;


// Should i create an enum? It's not really an enumeration of possible value... 
enum class IOPort : uint8_t {
	channel_0_data = 0x40, // read/write
	channel_1_data = 0x41, // read/write
	channel_2_data = 0x42, // read/write
	mode_or_command_register = 0x43 // (Write only, a read is ignored)
};

enum class Bit_76 {
	channel_0 = 0b00, 
	channel_1 = 0b01,
	channel_2 = 0b10, 
	read_back_command = 0b11, 
};



} // namespace pit
