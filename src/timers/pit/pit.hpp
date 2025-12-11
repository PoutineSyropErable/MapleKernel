#pragma once
#include "pit.h"
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace pit
{

constexpr uint32_t PIT_FREQ_HZ = 1'193'182;

// This is techically max -1. This number is uint16_t max.
constexpr uint16_t MAX_FREQ_DIVIDER = 65535; // 0 is mapped to 65536. (Some weird hardware might not. Fuck them)
constexpr float	   max_single_wait	= (float)(MAX_FREQ_DIVIDER + 1) / (float)(PIT_FREQ_HZ);

constexpr uint8_t PIT_IRQ_CHANEL0 = 0;

// Should i create an enum? It's not really an enumeration of possible value...
enum class IOPort : uint8_t
{
	channel_0_data_port		 = 0x40, // 0x40: read/write
	channel_1_data			 = 0x41, // 0x41: read/write
	channel_2_data			 = 0x42, // 0x42: read/write
	mode_or_command_register = 0x43	 // 0x43: (Write only, a read is ignored)
};

namespace MCR
{
enum class Bit_6_7 : uint8_t
{
	channel_0		  = 0b00,
	channel_1		  = 0b01,
	channel_2		  = 0b10,
	read_back_command = 0b11, //(8254+ only, So supported by anything modern)
};

// This
enum class Bit_4_5 : uint8_t
{
	latch_count_get_value_cmd = 0b00,
	low_byte_only			  = 0b01,
	high_byte_only			  = 0b10,
	low_then_high_byte		  = 0b11,
};

struct __attribute__((packed)) split_uint16_t
{
	uint8_t low;
	uint8_t high;
};

#define PIT_INCLUDE_ALIAS_MODES
enum class Bit_1_3 : uint8_t
{
	single_shot			  = 0b000,
	hardware_one_shot	  = 0b001, //  hardware re-triggerable one-shot)
	rate_generator		  = 0b010, //
	square_wave_generator = 0b011,
	software_strobe		  = 0b100,
	hardware_strobe		  = 0b101,
#ifdef PIT_INCLUDE_ALIAS_MODES
	// Should I even use these bellow?
	// They are the same way to send the same command
	// Used for readbacks so needed?
	// If not needed, it will polute switch statement
	rate_generator_2		= 0b110,
	square_wave_generator_2 = 0b111,
#endif
};

enum class Bit_0 : bool
{
	binary16		= 0,
	four_digits_bcd = 1, // Stuff work with information saved as 4 digits. Not supported a lot.
						 // Let's not program arround it
};

} // namespace MCR

struct mode_command_register
{
	enum MCR::Bit_0	  bcd_binary_mode : 1;
	enum MCR::Bit_1_3 operating_mode : 3;
	enum MCR::Bit_4_5 access_mode : 2;
	enum MCR::Bit_6_7 channel : 2;

	constexpr operator uint8_t() const noexcept
	{
		union
		{
			mode_command_register c;
			uint8_t				  raw;
		} u{.c = *this};
		return u.raw;
	}
};

STATIC_ASSERT(sizeof(mode_command_register) == 1, "Must be 1 byte");

enum class PIT_Error
{
	None = 0,
};

// =============== End
int send_wait_count_command(uint32_t freq_divider_count);
int wait(float seconds);

} // namespace pit
