#pragma once
#include <stddef.h>
#include <stdint.h>

namespace apic
{

struct apic_support
{
	bool apic : 1;
	bool x2apic : 1;
	bool tsc_deadline : 1;
};

struct apic_support has_apic();

enum class error : uint8_t
{
	none = 0,
	boot_core_timeout,
	bad_time,
	timeout_sending_ipi,
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

constexpr uintptr_t lapic_address = (0xFEE0'0000);

constexpr uint8_t number_of_reserved_interrupt = 32;
constexpr uint8_t spurious_interrupt_vector	   = 56; // 32 + 24 = reserved interrupt + 24 irqs per apic.
constexpr uint8_t interrupt_entered_main	   = 57;

void calibrate_lapic_timer();

enum apic::error wake_core(uint8_t core_id, void (*core_bootstrap)(), void (*core_main)());
void			 wait_till_interrupt(uint8_t interrupt_number);
enum apic::error send_ipi(uint8_t core_id, uint8_t int_vector);
void			 send_eoi();

} // namespace apic
