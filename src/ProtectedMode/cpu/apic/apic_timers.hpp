#pragma once
#include "stdint.h"

#include "apic.hpp"
#include "apic_io.hpp"

// Temporary apic internal include
#include "apic_internals.hpp"

namespace apic_timer
{

constexpr uint8_t apic_wait_interrupt	   = 58; //
constexpr uint8_t apic_timer_fn1_interrupt = 59; // I'mma reserved these right now
constexpr uint8_t apic_timer_fn2_interrupt = 60; //
void			  start_timer(uint8_t vector, uint32_t initial_count, enum apic::divide_configuration::type divide_configuration,
				 enum apic::timer_mode::type timer_mode);
} // namespace apic_timer
