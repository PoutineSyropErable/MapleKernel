#include "apic_io.hpp"
#include "apic_io_internals.hpp"
// #include "pic_ps2.h"

namespace apic_io
{

ApicIO apic_io;
void   init_io_apic(uint8_t *irq_to_gsi)
{
	uint8_t cur_owner = apic_io.get_apic_id_of_owner();
	kprintf("The current ownser is %u\n", cur_owner);
#define BSP_APIC_ID 0
	if (cur_owner != BSP_APIC_ID)
	{
		apic_io.set_apic_id_of_owner(0);
	}

	version v					  = apic_io.get_version_and_max_red();
	uint8_t version				  = v.version;
	uint8_t max_redirection_entry = v.max_redirection_entry;
	kprintf("Version = %u, Max Redirection Entry = %u\n", version, max_redirection_entry);

	redirection_entry_high pit_high{.destination = {.phyiscal_apic_id = 0}};
	redirection_entry_low  pit_low{.vector = 32, .mask = mask::enable};
	apic_io.write_redirection(irq_to_gsi[0], pit_low, pit_high);

	redirection_entry_high keyboard_high{.destination = {.phyiscal_apic_id = 0}};
	redirection_entry_low  keyboard_low{.vector = 33, .mask = mask::enable};
	apic_io.write_redirection(irq_to_gsi[1], keyboard_low, keyboard_high);

	redirection_entry_high mouse_high{.destination = {.phyiscal_apic_id = 0}};
	redirection_entry_low  mouse_low{.vector = 44, .mask = mask::enable};
	apic_io.write_redirection(irq_to_gsi[12], mouse_low, mouse_high);
}

uint8_t get_max_redirection_entry_count()
{
	return apic_io.get_version_and_max_red().max_redirection_entry;
}

uint8_t ApicIO::get_apic_id_of_owner()
{
	register_select.write(RegisterOffsets::id);
	union io_window win = io_window.read();
	return win.id.apic_id;
}

void ApicIO::set_apic_id_of_owner(uint8_t apic_id_of_owner)
{
	// I checked code and it's good
#ifdef DEBUG
	assert(apic_id_of_owner < 0b1111, "Can't set a apic id too big. Max 15\n");
#endif
	register_select.write(RegisterOffsets::id);

	id				owner_id{.apic_id = apic_id_of_owner};
	union io_window owner_id_u{.id = owner_id};
	io_window.write(owner_id_u);
	// Mmio write to a union could be bad
}

version ApicIO::get_version_and_max_red()
{
	register_select.write(RegisterOffsets::version);
	union io_window win = io_window.read();
	return win.version;
}

uint8_t ApicIO::get_arbitration()
{
	register_select.write(RegisterOffsets::arbitration);
	union io_window win = io_window.read();
	return win.arbitartion.arbitration_id;
}

struct HighAndLow
{
	RegisterOffsets low;
	RegisterOffsets high;
};
static inline HighAndLow get_offsets(uint8_t red_idx)
{
	uint32_t   offset_raw_low  = (uint32_t)RegisterOffsets::relocation_table_base + red_idx * 2;
	uint32_t   offset_raw_high = offset_raw_low + 1;
	HighAndLow ret{.low = (RegisterOffsets)offset_raw_low, .high = (RegisterOffsets)offset_raw_high};
	return ret;
}

void ApicIO::write_redirection(uint8_t irq, redirection_entry_low red_low, redirection_entry_high red_high)
{
	HighAndLow hl = get_offsets(irq);

	// I'm not sure of the proper write order
	register_select.write(hl.low);
	union io_window rl{.red_low = red_low};
	io_window.write(rl);

	register_select.write(hl.high);
	union io_window rh{.red_high = red_high};
	io_window.write(rh);
}
void ApicIO::read_redirection_ws(uint8_t irq, redirection_entry_low &red_low, redirection_entry_high &red_high)
{

	HighAndLow hl = get_offsets(irq);

	register_select.write(hl.low);
	union io_window low_u = io_window.read();
	red_low				  = low_u.red_low;

	register_select.write(hl.high);
	union io_window high_u = io_window.read();
	red_high			   = high_u.red_high;
}

full_redirection_entry ApicIO::read_redirection(uint8_t irq)
{

	HighAndLow			   hl = get_offsets(irq);
	full_redirection_entry ret;

	register_select.write(hl.low);
	union io_window low_u = io_window.read();
	ret.red_low			  = low_u.red_low;

	register_select.write(hl.high);
	union io_window high_u = io_window.read();
	ret.red_high		   = high_u.red_high;
	return ret;
}

} // namespace apic_io
