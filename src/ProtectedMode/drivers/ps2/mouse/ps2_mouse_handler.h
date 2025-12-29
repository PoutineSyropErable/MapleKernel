#pragma once
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif
	void mouse_handler(uint8_t scancode, uint8_t port_number);

	struct mouse_properties
	{
		int16_t x;
		int16_t y;
		int16_t z; // scroll wheel

		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};

	extern volatile struct mouse_properties g_mouse_prop;

#ifdef __cplusplus
}
#endif
