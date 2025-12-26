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

#define PIT_MAX_MSG 32
	extern volatile bool	  pit_is_new_timeout;
	extern volatile uint8_t	  pit_write_index;
	extern volatile uint32_t *pit_msg_address[PIT_MAX_MSG];
	extern volatile uint32_t  pit_msg_value[PIT_MAX_MSG];

	// =============== End
#ifdef __cplusplus
}
#endif
