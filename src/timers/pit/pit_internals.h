// pit_internals.h
#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	// ================ Start

	// because shared by code and isr, need volatile
	extern volatile bool pit_interrupt_handled;

	// =============== End
#ifdef __cplusplus
}
#endif
