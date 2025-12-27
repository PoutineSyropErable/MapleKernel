#pragma once
#include <stddef.h>
#include <stdint.h>

namespace apic
{

struct apic_support
{
	bool apic : 1;
	bool x2apic : 1;
};

struct apic_support has_apic();

enum class error
{
	none = 0,
	boot_core_timeout,
	bad_time,
};

// APIC initiation itself
enum apic::error init_apic();

// The lapic initiation (For the running core)
enum apic::error init_lapic();
enum apic::error init_io_apic();
uint32_t		 get_base_address();
void			 set_base_address(uint32_t base_address);

/*
Method using lapic.
*/
uint8_t get_core_id();

/*
method using fs/gs.
*/
uint8_t get_core_id_fast();

constexpr uintptr_t lapic_address	= (0xFEE0'0000);
constexpr uintptr_t io_apic_address = (0xFEC0'0000);

void calibrate_lapic_timer();

enum apic::error wake_core(uint8_t core_id, void (*core_bootstrap)(), void (*core_main)());
void			 wait_till_interrupt(uint8_t interrupt_number);

} // namespace apic
