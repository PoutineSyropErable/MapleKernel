#pragma once
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif
	void mouse_handler(uint8_t scancode, uint8_t port_number);

	struct mouse_pos
	{
		int16_t x;
		int16_t y;
		int16_t z; // scroll wheel
	};

	extern volatile struct mouse_pos g_mouse_pos;

#ifdef __cplusplus
}
#endif
