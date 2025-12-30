#pragma once

#include <stddef.h>
#include <stdint.h>

namespace apic_io
{
constexpr uintptr_t io_apic_address		  = (0xFEC0'0000);
constexpr uint8_t	number_of_apic_io_irq = 24;

void	init_io_apic(uint8_t *irq_to_gsi);
uint8_t get_max_redirection_entry_count();
} // namespace apic_io
