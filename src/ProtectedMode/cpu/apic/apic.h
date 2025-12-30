#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	// Stuff here

	uint8_t apic_get_core_id();
	void	apic_send_eoi();

#ifdef __cplusplus
}
#endif
