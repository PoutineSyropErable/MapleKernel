#include "apic.hpp"
#include "apic_internals.hpp"
#include "apic_io.hpp"
#include "assert.h"
#include "cast.h"
#include "intrinsics.h"
// This file needs -fno-strict-aliasing
#include "multicore.h"

using namespace apic;
namespace apic_timer
{
static LapicRegisters lapic;

void start_timer(
	uint8_t vector, uint32_t initial_count, enum divide_configuration::type divide_configuration, enum timer_mode::type timer_mode)
{
#ifdef DEBUG
	assert(timer_mode <= timer_mode::tsc_deadline, "Invalid timer mode");

	if (!support_tsc_deadline && timer_mode == timer_mode::tsc_deadline)
	{
		abort_msg("Tsc deadline not supported, don't try to use it\n");
	}
#endif

	lapic.divide_configuration.write({.divide_configuration = divide_configuration});

	lapic.lvt_timer.write({.vector_number = vector, .mask = mask::enable, .timer_mode = timer_mode});

	lapic.initial_count.write({.value = initial_count});
	// The proper order is:
	// Divide -> timer -> Initial count
}

} // namespace apic_timer
