#include "apic.hpp"
#include "apic_internals.hpp"
#include "apic_io.hpp"
#include "assert.h"
#include "cast.h"
#include "intrinsics.h"
// This file needs -fno-strict-aliasing
#include "apic_timers.hpp"
#include "cpuid.hpp"
#include "cpuid_results.hpp"
#include "math.hpp"
#include "multicore.h"
#include "pit.hpp"
#include "pit_interrupt_handler.hpp"
#include "pit_quick.hpp"

using namespace apic;
static LapicRegisters lapic;
namespace apic_timer
{

uint64_t apic_frequency = 0;

void start_timer(uint8_t vector, uint32_t initial_count, enum divide_configuration::type divide_configuration,
	enum timer_mode::type timer_mode, mask::type mask = mask::enable)
{
#ifdef DEBUG
	assert(timer_mode <= timer_mode::tsc_deadline, "Invalid timer mode");

	if (!support_tsc_deadline && timer_mode == timer_mode::tsc_deadline)
	{
		abort_msg("Tsc deadline not supported, don't try to use it\n");
	}
#endif

	lapic.divide_configuration.write({.divide_configuration = divide_configuration});

	lapic.lvt_timer.write({.vector_number = vector, .mask = mask, .timer_mode = timer_mode});

	lapic.initial_count.write({.value = initial_count});
	// The proper order is:
	// Divide -> timer -> Initial count
}

uint32_t get_tsc_frequency()
{
	struct cpuid::cpuid_verified_result cpuid_87 = cpuid::call_cpuid(cpuid::CpuidFunction::AdvancedPowerMgmt, {.raw = 0});
	if (cpuid_87.has_error())
	{
		kprintf("Couldn't use cpuid h800...7\n");
		return 0;
	}

	uint32_t		  edx					  = cpuid_87.regs.edx;
	constexpr uint8_t invariant_tsc_bit_index = 8;
	edx &= (1 < invariant_tsc_bit_index);
	if (!edx)
	{
		kprintf("The TSC isn't invariant\n");
		return 1;
	}

	struct cpuid::cpuid_verified_result cpuid_15 = cpuid::call_cpuid(cpuid::CpuidFunction::TSC_INFO, {.raw = 0});
	if (cpuid_15.has_error())
	{
		kprintf("Couldn't use cpuid h15\n");
		return 2;
	}

	return 0xFFFF;
}

uint32_t sync_apic_with_pit()
{

	uint32_t tsc_freq = get_tsc_frequency();
	if (tsc_freq > 10)
	{

		kprintf("tsc freq = %u\n\n", tsc_freq);
	}

	pit_ih::set_quick_path_mode(1);

	uint32_t start_count = 0xFFFF'FFFF;
	start_timer(apic_sync_interrupt, start_count, divide_configuration::divide_by_1, timer_mode::single_shot, mask::disable);
	pit::wait_pit_count_precise(0); // 65536
	// pit::wait_pit_count(65536); // 65536
	// Slower on qemu if i get the start count right after start_timer. Equal on my machine

	current_count_register cc	= lapic.current_count.read();
	uint32_t			   diff = start_count - cc.value;

	apic_frequency = PIT_FREQ_HZ * (diff / 65536);
	uint32_t ratio = diff / 65536;
	kprintf("The APIC is %u faster then the pit: \n", ratio);
	kprintf("APIC frequency: %u Hz (No comparse)\n", apic_frequency);

#ifdef QEMU
	constexpr uint32_t qemu_apic_freq = 1'000'000'000;
	uint32_t		   ghz_diff		  = abs((int)apic_frequency - (int)qemu_apic_freq);
	if (ghz_diff != 0)
	{
		kprintf("\n");
		kprintf("ghz_diff : %u\n", ghz_diff);
		uint32_t pos_ratio = qemu_apic_freq / ghz_diff;
		kprintf("Pos ratio : %u\n", pos_ratio);
		if (pos_ratio > 100)
		{
			apic_frequency = qemu_apic_freq;
		}
	}

	kprintf("APIC frequency: %u Hz\n", apic_frequency);
#endif

	if (tsc_freq > 10)
	{
		kprintf("TSC frequency: %u Hz\n", tsc_freq);
		kprintf("\n");

		// Basic comparisons
		kprintf("APIC - TSC: %d Hz\n", apic_frequency - tsc_freq);
		kprintf("TSC - APIC: %d Hz\n", tsc_freq - apic_frequency);
		kprintf("\n");

		double apic_over_tsc = (double)apic_frequency / tsc_freq;
		kprintf("APIC / TSC ratio: %f.6\n", apic_over_tsc);

		double tsc_over_apic = (double)tsc_freq / apic_frequency;
		kprintf("TSC / APIC ratio: %f.6 \n", tsc_over_apic);
		kprintf("\n");

		double diff_abs			= (double)abs((int64_t)apic_frequency - (int64_t)tsc_freq);
		double pct_diff_to_tsc	= (diff_abs / tsc_freq) * 100.0;
		double pct_diff_to_apic = (diff_abs / apic_frequency) * 100.0;

		kprintf("Absolute difference: %f.0 Hz\n", diff_abs);
		kprintf("Difference as perc of TSC: %f.3 \n", pct_diff_to_tsc);
		kprintf("Difference as perc of APIC: %f.3 \n", pct_diff_to_apic);
		kprintf("\n");
	}

	// MHz display for easier reading
	kprintf("\n");
	kprintf("APIC: %f.3 MHz\n", apic_frequency / 1000000.0);
	kprintf("PIT: %f.3 MHz\n", PIT_FREQ_HZ / 1000000.0);
	if (tsc_freq > 10)
	{
		kprintf("TSC: %f.3 MHz\n", tsc_freq / 1000000.0);
	}
	return apic_frequency;
}

} // namespace apic_timer
