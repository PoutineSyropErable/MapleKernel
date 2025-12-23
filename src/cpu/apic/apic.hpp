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
};

enum apic::error init_apic();
enum apic::error init_io_apic();

/*
Method using lapic.
*/
uint8_t get_core_id();

/*
method using fs/gs.
*/
uint8_t get_core_id_fast();

extern volatile void *lapic_address;
extern volatile void *io_appic_address;

void calibrate_lapic_timer();

extern "C" void core_bootstrap();
void			core_main();
void			wake_core(uint8_t core_id, void core_bootstrap(), void core_main());
void			wait_till_interrupt(uint8_t interrupt_number);

} // namespace apic
