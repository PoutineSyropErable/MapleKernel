#pragma once
#include "apic.hpp"
#include "intrinsics.h"
#include "special_pointers.hpp"
#include "static_assert.h"
#include "std.hpp"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

namespace apic
{

constexpr uint8_t apic_msr = 0x1B;

struct apic_msr_eax
{
	uint32_t reserved0 : 8;				 // 0–7
	uint32_t is_bootstrap_processor : 1; // 8, bsp, read only
	uint32_t reserved1 : 1;				 // 9
	uint32_t x2apic : 1;				 // 10
	uint32_t apic_enable : 1;			 // 11
	uint32_t base_low_page_idx : 20;	 // 12–31, base_low = base_low_page_idx << 12
};
STATIC_ASSERT(sizeof(apic_msr_eax) == 4, "Must be 32 bit");
STATIC_ASSERT(alignof(apic_msr_eax) == 4, "Must be 32 bit");

struct apic_msr_edx
{
	uint32_t base_high : 4; // 32–35
	uint32_t reserved : 28; // 36–63
};
STATIC_ASSERT(sizeof(apic_msr_edx) == 4, "Must be 32 bit");
STATIC_ASSERT(alignof(apic_msr_edx) == 4, "Must be 32 bit");

void read_apic_msr(apic_msr_eax *eax, apic_msr_edx *edx);
void write_apic_msr(apic_msr_eax eax, apic_msr_edx edx);

enum class lapic_registers_offset : uint16_t
{
	lapic_id	  = 0x20, // 0x20, Read/Write
	lapic_version = 0x30, // 0x30, Read only

	task_priority			  = 0x80, // 0x80, Read/Write
	arbitration_priority	  = 0x90, // 0x90, Read only
	process_priority		  = 0xA0, // 0xA0, Read only
	end_of_interrupt		  = 0xB0, // 0xB0, Write only
	remote_read				  = 0xC0, // 0xC0, Read Only
	logical_destination		  = 0xD0, // 0xD0, Read/Write
	destination_format		  = 0xE0, // 0xE0, Read/Write
	spurious_interrupt_vector = 0xF0, // 0xF0, Read/Write

	in_service_0 = 0x100, // 0x100, Read Only
	in_service_1 = 0x110, // 0x110, Read Only
	in_service_2 = 0x120, // 0x120, Read Only
	in_service_3 = 0x130, // 0x130, Read Only
	in_service_4 = 0x140, // 0x140, Read Only
	in_service_5 = 0x150, // 0x150, Read Only
	in_service_6 = 0x160, // 0x160, Read Only
	in_service_7 = 0x170, // 0x170, Read Only

	trigger_mode_0 = 0x180, // 0x180, Read only
	trigger_mode_1 = 0x190, // 0x190, Read only
	trigger_mode_2 = 0x1a0, // 0x1a0, Read only
	trigger_mode_3 = 0x1b0, // 0x1b0, Read only
	trigger_mode_4 = 0x1c0, // 0x1c0, Read only
	trigger_mode_5 = 0x1d0, // 0x1d0, Read only
	trigger_mode_6 = 0x1e0, // 0x1e0, Read only
	trigger_mode_7 = 0x1f0, // 0x1f0, Read only

	interrupt_request_0 = 0x200, // 0x200, Read only
	interrupt_request_1 = 0x210, // 0x210, Read only
	interrupt_request_2 = 0x220, // 0x220, Read only
	interrupt_request_3 = 0x230, // 0x230, Read only
	interrupt_request_4 = 0x240, // 0x240, Read only
	interrupt_request_5 = 0x250, // 0x250, Read only
	interrupt_request_6 = 0x260, // 0x260, Read only
	interrupt_request_7 = 0x270, // 0x270, Read only

	error_status = 0x280, // 0x280, Read Only
	lvt_cmci	 = 0x2f0, // 0x2f0, Read/Write, LVT Corrected Machine Check Interrupt (CMCI) Register

	command_low	 = 0x300, // 0x300, Read/Write
	command_high = 0x310, // 0x310, Read/Write

	lvt_timer							= 0x320, // 0x320, Read/Write
	lvt_thermal_sensor					= 0x330, // 0x330, Read/Write
	lvt_performance_monitoring_counters = 0x340, // 0x340, Read/Write
	lvt_lint0							= 0x350, // 0x350, Read/Write
	lvt_lint1							= 0x360, // 0x360, Read/Write
	lvt_error							= 0x370, // 0x370, Read/Write

	initial_count = 0x380, // 0x380, Read/Write
	current_count = 0x390, // 0x390, Read Only

	divide_configuration_register = 0x3E0, // 0x3E0, Read / Write

};

/* ============================= Special structs for important registers =============================*/

struct __attribute__((packed, aligned(4))) lapic_version_register
{
	uint8_t version;
	uint8_t reserved;
	uint8_t max_lvt_entry_idt;
	uint8_t reserved2;
};

struct __attribute__((packed, aligned(4))) lapic_id_register
{
	uint32_t reserved : 24 = 0;
	uint8_t	 apic_id;
};

struct lapic_priority_register
{
	uint32_t reserved0 : 4;	 // bits 0–3 (must be 0)
	uint32_t priority : 4;	 // bits 4–7 (TPRI)
	uint32_t reserved1 : 24; // bits 8–31 (must be 0)
};

STATIC_ASSERT(sizeof(lapic_version_register) == 4, "lapic_version_register  must be 32 bit");
STATIC_ASSERT(alignof(lapic_version_register) == 4, "lapic_version_register  must be 32 bit");

STATIC_ASSERT(sizeof(lapic_priority_register) == 4, "lapic priority register  must be 32 bit");
STATIC_ASSERT(alignof(lapic_priority_register) == 4, "lapic priority register  must be 32 bit");

namespace task_priority
{

enum type : uint8_t
{
	all	 = 0b0000,
	none = 0b1111,
};

}

namespace task_priority_subclass
{
enum type : uint8_t
{
	default_ = 0b0000,
	// :TODO:

};

} // namespace task_priority_subclass

struct task_priority_register
{
	enum task_priority_subclass::type task_priority_subclass : 4 = task_priority_subclass::default_;
	enum task_priority::type		  task_priority : 4			 = task_priority::all;
	uint32_t						  res : 24					 = 0;
};

STATIC_ASSERT(sizeof(task_priority_register) == 4, "Task priority must be 32 bit");
STATIC_ASSERT(alignof(task_priority_register) == 4, "Task priority must be 32 bit");

/* =============== Command registers ==================== */

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
	start_up		= 0b110
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

namespace level
{
enum type : bool
{
	deassert = 0,
	assert	 = 1
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

namespace remote_read_status
{
enum type : uint8_t
{
	invalid = 0b00,
	pending = 0b01,
	done	= 0b10 // Delivery done, data available
};
}

namespace destination_type
{
enum type : uint8_t
{
	normal			   = 0b00,
	self			   = 0b01,
	all_including_self = 0b10,
	all_excluding_self = 0b11
};
}

struct __attribute__((packed, aligned(4))) interrupt_command_low_register
{
	uint8_t						vector_number;
	enum delivery_mode::type	delivery_mode : 3	 = delivery_mode::fixed;
	enum destination_mode::type destination_mode : 1 = destination_mode::physical;
	// Phyiscal: The apic id (command_high) ==  Single Apic id, Logical: The Apic group id.
	bool delivery_status_pending_ro : 1 = 0; // cleared when the interrupt has been accepted by the target. Always wait till cleared
											 // delivery status: Read only
	bool			 _reserved : 1 = 0;
	enum level::type level : 1 = level::assert; // only one who default to non zero value (This bit is ignored for edge. Only used in level)
	enum trigger_mode::type		  trigger_mode : 1		 = trigger_mode::edge;
	enum remote_read_status::type remote_read_status : 2 = remote_read_status::invalid; // bit 16-17.
	enum destination_type::type	  destination_type : 2	 = destination_type::normal;	// bit 18-19
	uint16_t					  _reserved3 : 12		 = 0;							// bit 20-31

	// We create this struct when we write, and when we write, delivery status and remote read status should be 0
};
STATIC_ASSERT(sizeof(interrupt_command_low_register) == 4, "ICR low must be 32 bit");
STATIC_ASSERT(alignof(interrupt_command_low_register) == 4, "ICR low must be 32 bit");

struct __attribute__((packed, aligned(4))) interrupt_command_high_register
{
	// TODO: Unfucked the default constructor here.
	// Make them not suck mega dick.
	// and just use a mov uint32_t.
	// Learn what these operators are.
	uint32_t reserved : 24 = 0;
	uint32_t local_apic_id_of_target : 4;
	uint32_t unused : 4 = 0;
	// Putting nothing else here makes the highest 4 bits reserved, and unaccessible

	// constexpr interrupt_command_high_register(uint32_t lapic_id = 0) : reserved(0), local_apic_id_of_target(lapic_id), unused(0)
	// {
	// }
};

STATIC_ASSERT(sizeof(interrupt_command_high_register) == 4, "ICR high must be 32 bit");
STATIC_ASSERT(alignof(interrupt_command_high_register) == 4, "ICR high must be 32 bit");

/* =============== Command registers ==================== */
struct __attribute__((packed, aligned(4))) spurious_interrupt_vector_register
{
	uint8_t	 vector;		   // bits 0-7
	bool	 apic_enable : 1;  // bit 8
	uint8_t	 reserved1 : 3;	   // bits 9-11
	bool	 eoi_suppress : 1; // bit 12
	uint32_t reserved2 : 19;   // bits 13-31
};
STATIC_ASSERT(sizeof(spurious_interrupt_vector_register) == 4, "SVR must be 32-bit");
STATIC_ASSERT(alignof(spurious_interrupt_vector_register) == 4, "SVR must be 32-bit");

/* ========================== Local Vector Tables ============================= */

namespace lvt_delivery_mode
{
enum type : uint8_t
{
	fixed							= 0b000,
	smi								= 0b010,
	nmi								= 0b100,
	external_interrupt_is_delivered = 0b111,
};
}

namespace delivery_status
{
enum type : bool
{
	successful_delivery = 0,
	pending				= 1,
	// When successful_delivery, The associated bit in the irr is set
};
}
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

namespace mask
{
enum type : bool
{
	enable	= 0b0,
	disable = 0b1,
};
}

namespace timer_mode
{
enum type : uint8_t
{
	single_shot	 = 0b00,
	repeat		 = 0b01,
	tsc_deadline = 0b10, // This mode needs support
						 // reserved	= 0b11,
};
}

struct __attribute__((aligned(4))) lvt_register_min
{
	uint8_t			vector_number;
	uint8_t			res : 7	  = 0;
	enum mask::type mask : 1  = mask::enable; // ||| enable = unmasked. So regular interrupts can pass
	uint16_t		res2 : 15 = 0;
};

struct __attribute__((aligned(4))) lvt_register_full
{
	uint8_t					   vector_number;								   // 0-7
	lvt_delivery_mode::type	   message_type : 3	   = lvt_delivery_mode::fixed; // message type == delivery mode (synoynms. diff docs) 8-10
	bool					   res1 : 1			   = 0;						   // 11. (Bit 11 is unused everywhere)
	enum delivery_status::type delivery_status : 1 = delivery_status::successful_delivery;	  // 12
	enum pin_polarity::type	   pin_polarity : 1	   = pin_polarity::active_high;				  // 13
	enum remote_interrupt_request::type remote_irr : 1	 = remote_interrupt_request::option0; // 14
	enum trigger_mode::type				trigger_mode : 1 = trigger_mode::edge;				  // 15
	enum mask::type						mask : 1		 = mask::enable;					  // 16
	enum timer_mode::type				timer_mode : 2	 = timer_mode::single_shot;			  // 17 - 18 (17 only if no tsc deadline)
	uint16_t							res3 : 13		 = 0;
};

struct __attribute__((aligned(4))) lvt_lint_register
{
	uint8_t					   vector_number;								   // 0-7
	lvt_delivery_mode::type	   message_type : 3	   = lvt_delivery_mode::fixed; // 8-10 message type == delivery mode (synoynms. diff docs)
	bool					   res1 : 1			   = 0;						   // 11.
	enum delivery_status::type delivery_status : 1 = delivery_status::successful_delivery;	  // 12
	enum pin_polarity::type	   pin_polarity : 1	   = pin_polarity::active_high;				  // 13
	enum remote_interrupt_request::type remote_irr : 1	 = remote_interrupt_request::option0; // 14
	enum trigger_mode::type				trigger_mode : 1 = trigger_mode::edge;				  // 15
	enum mask::type						mask : 1		 = mask::enable;					  // 16
	uint16_t							res3 : 15		 = 0;								  // 17-31
};

extern bool support_tsc_deadline;
struct __attribute__((aligned(4))) lvt_timer_register
{
	uint8_t					   vector_number;											   // 0-7
	bool					   res1 : 4			   = 0;									   // 8-11
	enum delivery_status::type delivery_status : 1 = delivery_status::successful_delivery; // 12
	uint8_t					   res2 : 3			   = 0;
	enum mask::type			   mask : 1			   = mask::enable;			  // 16
	enum timer_mode::type	   timer_mode : 2	   = timer_mode::single_shot; // 17-18
	uint16_t				   res3 : 13		   = 0;						  // 19-31
};

STATIC_ASSERT(sizeof(lvt_register_min) == 4, "LVT must be 32-bit");
STATIC_ASSERT(sizeof(lvt_register_full) == 4, "LVT must be 32-bit");
STATIC_ASSERT(sizeof(lvt_timer_register) == 4, "LVT must be 32-bit");

STATIC_ASSERT(alignof(lvt_register_min) == 4, "LVT must be 32-bit");
STATIC_ASSERT(alignof(lvt_register_full) == 4, "LVT must be 32-bit");
STATIC_ASSERT(alignof(lvt_timer_register) == 4, "LVT Timer must be 32-bit aligned");

struct current_count_register
{
	uint32_t value;
};

struct initial_count_register
{
	uint32_t value;
};

STATIC_ASSERT(sizeof(current_count_register) == 4, "Current Count register must be 32-bit in size");
STATIC_ASSERT(alignof(current_count_register) == 4, "Current Count register must be 32-bit in size");

STATIC_ASSERT(sizeof(initial_count_register) == 4, "Initial Count register must be 32-bit in size");
STATIC_ASSERT(alignof(initial_count_register) == 4, "Initial Count register must be 32-bit in size");

namespace
{
// Information taken from the amd manual
// But i'll implement it with a fixed 0 in the other bit
enum class divide_configuration_amd : uint8_t
{
	// The value in here are the first two bits and the 4th
	// Bit combining the create the 3 bits
	// divide by = 2^[(n + 1) % 8]
	divide_by_2	  = 0b000,
	divide_by_4	  = 0b001,
	divide_by_8	  = 0b010,
	divide_by_16  = 0b011,
	divide_by_32  = 0b100,
	divide_by_64  = 0b101,
	divide_by_128 = 0b110,
	divide_by_1	  = 0b111,
	// Added purely to make the pattern matching easier
	// For human to understand
};

struct divide_configuration_register_amd
{
	// This ones exist only to show that bit 3 is reserved.
	uint32_t first_two : 1;
	uint32_t reserved : 1 = 0;
	uint32_t third_at_bit_4 : 1;
	uint32_t rest_reserved : 29 = 0;
};
STATIC_ASSERT(sizeof(divide_configuration_register_amd) == 4, "div must be 32 size");
STATIC_ASSERT(alignof(divide_configuration_register_amd) == 4, "div must be 32 size");
} // namespace

namespace divide_configuration
{
enum type : uint8_t
{
	// The 3rd bit must be 0
	// The 4th bit of this actually represent the third bit
	divide_by_2	  = 0b0000,
	divide_by_4	  = 0b0001,
	divide_by_8	  = 0b0010,
	divide_by_16  = 0b0011,
	divide_by_32  = 0b1000,
	divide_by_64  = 0b1001,
	divide_by_128 = 0b1010,
	divide_by_1	  = 0b1011,
};

}

struct divide_configuration_register
{
	enum divide_configuration::type divide_configuration : 4;
	uint32_t						reserved : 28 = 0;
};

STATIC_ASSERT(sizeof(divide_configuration_register) == 4, "div must be 32 size");
STATIC_ASSERT(alignof(divide_configuration_register) == 4, "div must be 32 size");
/* ============================= Constexpr helpers =============================*/

template <typename ToType> constexpr volatile ToType *get_mmio_address(volatile void *base, lapic_registers_offset offset)
{
	return reinterpret_cast<volatile ToType *>(reinterpret_cast<uintptr_t>(base) + (uint16_t)offset);
}

template <typename ToType> constexpr const std::mmio_ptr<ToType> get_const_mmio_ptr(volatile void *base, lapic_registers_offset offset)
{
	const std::mmio_ptr<ToType> ptr(get_mmio_address<ToType>(base, offset));
	return ptr;
}

template <typename ToType> constexpr std::mmio_ptr<ToType> get_mmio_ptr(uintptr_t base, lapic_registers_offset offset)
{
	return std::mmio_ptr<ToType>(base + static_cast<uint16_t>(offset));
}

template <typename ToType> constexpr std::mmio_ptr_ro<ToType> get_mmio_ptr_ro(uintptr_t base, lapic_registers_offset offset)
{
	return std::mmio_ptr_ro<ToType>(base + static_cast<uint16_t>(offset));
}

template <typename ToType> constexpr std::mmio_ptr_wo<ToType> get_mmio_ptr_wo(uintptr_t base, lapic_registers_offset offset)
{
	return std::mmio_ptr_wo<ToType>(base + static_cast<uint16_t>(offset));
}

constexpr enum lapic_registers_offset add_offset(lapic_registers_offset lapic_offset, uint8_t offset)
{
	uint8_t loff = (uint8_t)lapic_offset + offset;
	return (lapic_registers_offset)loff;
}

/* ============================= The Class =============================*/
class LapicRegisters
{
  public:
	// ------------------------------------------------------------------
	// Read-only / Read-write basic registers
	// ------------------------------------------------------------------
	static constexpr std::mmio_ptr_ro<lapic_id_register> lapic_id{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lapic_id)};
	static constexpr std::mmio_ptr<lapic_version_register> lapic_version{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lapic_version)};
	static constexpr std::mmio_ptr<struct task_priority_register> task_priority{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::task_priority)};
	static constexpr std::mmio_ptr_ro<uint32_t> arbitration_priority{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::arbitration_priority)};
	static constexpr std::mmio_ptr_ro<uint32_t> process_priority{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::process_priority)};

	// ------------------------------------------------------------------
	// Destination / formatting
	// ------------------------------------------------------------------
	static constexpr std::mmio_ptr_wo<uint32_t> remote_read{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::remote_read)};
	static constexpr std::mmio_ptr<uint32_t>	logical_destination{
		   lapic_address + static_cast<uintptr_t>(lapic_registers_offset::logical_destination)};
	static constexpr std::mmio_ptr<uint32_t> destination_format{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::destination_format)};
	static constexpr std::mmio_ptr<spurious_interrupt_vector_register> spurious_interrupt_vector{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::spurious_interrupt_vector)};

	// ------------------------------------------------------------------
	// ISR / TMR / IRR (arrays)
	// ------------------------------------------------------------------
	static constexpr std::mmio_ptr_ro<uint32_t> in_service[8] = {
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_0)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_1)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_2)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_3)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_4)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_5)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_6)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::in_service_7)},
	};

	// Trigger Mode Registers
	static constexpr std::mmio_ptr_ro<uint32_t> trigger_mode[8] = {
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_0)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_1)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_2)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_3)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_4)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_5)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_6)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::trigger_mode_7)},
	};

	// Interrupt Request Registers
	static constexpr std::mmio_ptr_ro<uint32_t> interrupt_request[8] = {
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_0)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_1)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_2)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_3)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_4)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_5)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_6)},
		std::mmio_ptr_ro<uint32_t>{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::interrupt_request_7)},
	};

	// ------------------------------------------------------------------
	// Error and LVT registers
	// ------------------------------------------------------------------
	static constexpr std::mmio_ptr_ro<uint32_t> error_status{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::error_status)};
	static constexpr std::mmio_ptr<uint32_t>	lvt_cmci{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lvt_cmci)};

	// ------------------------------------------------------------------
	// Interrupt command registers
	// ------------------------------------------------------------------
	static constexpr std::mmio_ptr<interrupt_command_low_register> command_low{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::command_low)};
	static constexpr std::mmio_ptr<interrupt_command_high_register> command_high{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::command_high)};

	// ------------------------------------------------------------------
	// Timer and LVT entries
	// ------------------------------------------------------------------
	static constexpr std::mmio_ptr<lvt_timer_register> lvt_timer{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lvt_timer)};

	static constexpr std::mmio_ptr<uint32_t> lvt_thermal_sensor{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lvt_thermal_sensor)};

	static constexpr std::mmio_ptr<uint32_t> lvt_performance_monitoring_counters{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lvt_performance_monitoring_counters)};

	static constexpr std::mmio_ptr<lvt_register_min> lvt_lint0{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lvt_lint0)};

	static constexpr std::mmio_ptr<lvt_register_min> lvt_lint1{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lvt_lint1)};

	static constexpr std::mmio_ptr<lvt_register_min> lvt_error{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lvt_error)};

	// ------------------------------------------------------------------
	// Timer counters and configuration
	// ------------------------------------------------------------------
	static constexpr std::mmio_ptr<current_count_register> initial_count{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::initial_count)};

	static constexpr std::mmio_ptr_ro<current_count_register> current_count{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::current_count)};

	static constexpr std::mmio_ptr<struct divide_configuration_register> divide_configuration{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::divide_configuration_register)};

	__attribute__((always_inline, fastcall)) inline void send_command(
		interrupt_command_low_register low, interrupt_command_high_register high)
	{
		command_high.write(high);
		command_low.write(low);
	}

	void send_eoi()
	{
		constexpr uint32_t eoi_safe_val = 0;
		// Writing something not 0 might cause a general protection fault
		end_of_interrupt.write(eoi_safe_val);
		// Since, lapic is core dependant, then we don't need to send this anywhere else. It's that easy
	}

	void test()
	{
		// get_mmio_ptr_ro((volatile void *)lapic_address, lapic_registers_offset::lapic_id);
		// Write first to high, then to low
		// uint32_t la							  = *lapic_id;
		// command_high->local_apic_id_of_target = 5;
		// interrupt_command_register_high ch{.local_apic_id_of_target = 5};
		// *command_high						= ch;
		interrupt_command_high_register ch2 = command_high.read();
		kprintf("ch2: %u\n", ch2.local_apic_id_of_target);
		// *command_high = ch2;
		// command_low->vector_number			= 1;
	}

  private:
	static constexpr std::mmio_ptr_wo<uint32_t> end_of_interrupt{
		get_mmio_ptr_wo<uint32_t>(lapic_address, lapic_registers_offset::end_of_interrupt)};
};

} // namespace apic
