#pragma once
#include "apic_io.hpp"
#include "special_pointers.hpp"
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace apic_io
{
// IO Register Select : IOREGSEL
// IO Register Window : IOREGWIN   || A register where you write the value. (It's a window to where you want to write)

enum class RegisterOffsets : uint32_t
{
	id			= 0x00,
	version		= 0x01,
	arbitration = 0x02,

	relocation_table_base = 0x10,
};
constexpr uintptr_t IOREGSEL = io_apic_address + 0x00;
constexpr uintptr_t IOWIN	 = io_apic_address + 0x10;

struct u64
{
	uint32_t low;
	uint32_t high;
};

struct id
{
	uint32_t reserved : 24 = 0;
	uint32_t apic_id : 4;
	// uint32_t reserved2 : 4;
};

struct version
{
	uint32_t version : 8;
	uint32_t reserved : 8 = 0;
	uint32_t max_redirection_entry : 8;
	// uint32_t reserved2 : 8;
};

struct arbitartion
{
	uint32_t reserved : 24 = 0;
	uint32_t arbitration_id : 4;
	// uint32_t reserved2 : 4;
};

namespace delivery_mode
{
enum type : uint8_t
{
	fixed			= 0b000,
	lowest_priority = 0b001,
	smi				= 0b010, // System Management Interrupt
	remote_read		= 0b011, // Intel reserved, AMD Remote Read
	nmi				= 0b100,
	init			= 0b101, // also used for init level deassert
	extINT			= 0b111
};
}

namespace destination_mode
{
enum type : bool
{
	physical = 0,
	logical	 = 1
};
}

namespace delivery_status
{
// If 0, the IRQ is just relaxed and waiting for something to happen (or it has fired and already processed by Local APIC(s)).
// If 1, it means that the IRQ has been sent to the Local APICs but it's still waiting to be delivered.
enum type : bool
{
	relaxed_or_already_processed = 0,
	arrived_but_waiting			 = 1
};
} // namespace delivery_status

namespace pin_polarity
{
enum type : bool
{
	active_high = 0b0,
	active_low	= 0b1,
};
}

namespace remote_interrupt_request
{
enum type : bool
{
	// TODO: Make sure of this
	option0 = 0b0,
	option1 = 0b1,
};
}

namespace trigger_mode
{
enum type : bool
{
	edge  = 0,
	level = 1
};
}

namespace mask
{
enum type : bool
{
	enable	= 0b0,
	disable = 0b1,
};
}

union destination
{
	uint8_t phyiscal_apic_id : 4;
	uint8_t logical_unknown : 8;
};

STATIC_ASSERT(sizeof(destination) == 1, "Must be 1 byte");
struct redirection_entry_low
{

	uint32_t							vector : 8;
	enum delivery_mode::type			delivery_mode : 3			 = delivery_mode::fixed;
	enum destination_mode::type			destination_mode : 1		 = destination_mode::physical;
	enum delivery_status::type			delivery_status : 1			 = delivery_status::relaxed_or_already_processed;
	enum pin_polarity::type				pin_polarity : 1			 = pin_polarity::active_high;
	enum remote_interrupt_request::type remote_interrupt_request : 1 = remote_interrupt_request::option0;
	enum trigger_mode::type				trigger_mode : 1			 = trigger_mode::edge;
	enum mask::type						mask : 1					 = mask::disable;
};

struct redirection_entry_high
{
	uint16_t		  r1 = 0;
	uint8_t			  r2 = 0;
	union destination destination;
};

struct full_redirection_entry
{
	redirection_entry_low  red_low;
	redirection_entry_high red_high;
};

STATIC_ASSERT(sizeof(version) == 4, "Version Must be 32 bit");
STATIC_ASSERT(sizeof(arbitartion) == 4, "Arbitration Must be 32 bit");
STATIC_ASSERT(sizeof(redirection_entry_low) == 4, "Red Low Must be 32 bit");
STATIC_ASSERT(sizeof(redirection_entry_high) == 4, "Red Low Must be 32 bit");

union io_window
{
	uint32_t					  raw = 0;
	struct id					  id;
	struct version				  version;
	struct arbitartion			  arbitartion;
	struct redirection_entry_low  red_low;
	struct redirection_entry_high red_high;
};

STATIC_ASSERT(sizeof(io_window) == 4, "Must be 32 bit");

class ApicIO

{
  private:
	static constexpr std::mmio_ptr_wo<RegisterOffsets> register_select{IOREGSEL};
	static constexpr std::mmio_ptr<union io_window>	   io_window{IOWIN};

	void	 write_single32(uint32_t index, uint32_t value);
	uint32_t read_single32(uint32_t index);

	void write_double32(uint32_t index, uint32_t value_low, uint32_t value_high);
	u64	 read_double32(uint32_t index);

  public:
	uint8_t				   get_apic_id_of_owner();
	void				   set_apic_id_of_owner(uint8_t apic_id_owner);
	version				   get_version_and_max_red();
	uint8_t				   get_arbitration();
	void				   write_redirection(uint8_t irq, redirection_entry_low red_low, redirection_entry_high red_high);
	full_redirection_entry read_redirection(uint8_t irq);
};

} // namespace apic_io
