#pragma once
#include "static_assert.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	// C stuff here

#ifdef __cplusplus
}
#endif

namespace ps2_mouse
{

struct __attribute__((packed)) first_packet
{
	bool button_left : 1   = 0;
	bool button_right : 1  = 0;
	bool button_middle : 1 = 0;
	bool always_one : 1	   = 1;
	bool x_axis_sign : 1;
	bool y_axis_sign : 1;
	bool x_axis_overflow : 1;
	bool y_axis_overflow : 1;

	// Default constructor (needed!)
	constexpr first_packet() noexcept = default;

	// uint8_t -> first_packet
	constexpr explicit first_packet(uint8_t raw) noexcept
		: button_left((raw >> 0) & 1), button_right((raw >> 1) & 1), button_middle((raw >> 2) & 1), always_one((raw >> 3) & 1),
		  x_axis_sign((raw >> 4) & 1), y_axis_sign((raw >> 5) & 1), x_axis_overflow((raw >> 6) & 1), y_axis_overflow((raw >> 7) & 1)
	{
	}

	// first_packet -> uint8_t
	constexpr operator uint8_t() const noexcept
	{
		return (button_left << 0) | (button_right << 1) | (button_middle << 2) | (always_one << 3) | (x_axis_sign << 4) |
			   (y_axis_sign << 5) | (x_axis_overflow << 6) | (y_axis_overflow << 7);
	}
};
STATIC_ASSERT(sizeof(first_packet) == 1, "first packet is size 1\n");

// For generic mouse. This isn't hardware. I'm just using it packed for memory setup.
struct __attribute__((packed)) generic_3_packet
{
	struct first_packet flags;
	uint8_t				x_axis_movement;
	uint8_t				y_axis_movement;
};
STATIC_ASSERT(sizeof(generic_3_packet) == 3, "should be well packed. Else, the mem move trick will fail\n");

} // namespace ps2_mouse
