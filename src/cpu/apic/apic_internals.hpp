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

struct apic_msr_edx
{
	uint32_t base_high : 4; // 32–35
	uint32_t reserved : 28; // 36–63
};
STATIC_ASSERT(sizeof(apic_msr_edx) == 4, "Must be 32 bit");

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

struct __attribute__((packed)) lapic_version_register
{
	uint8_t version;
	uint8_t reserved;
	uint8_t max_lvt_entry_idt;
	uint8_t reserved2;
};

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

struct __attribute__((packed)) interrupt_command_low_register
{
	uint8_t						vector_number;
	enum delivery_mode::type	delivery_mode : 3 = delivery_mode::fixed;
	enum destination_mode::type destination_mode : 1;
	// Phyiscal: The apic id (command_high) ==  Single Apic id, Logical: The Apic group id.
	bool delivery_status_pending_ro : 1 = 1; // cleared when the interrupt has been accepted by the target. Always wait till cleared
											 // delivery status: Read only
	bool						  _reserved : 1			 = 0;
	enum level::type			  level : 1				 = level::assert;
	enum trigger_mode::type		  trigger_mode : 1		 = trigger_mode::edge;
	enum remote_read_status::type remote_read_status : 2 = remote_read_status::pending; // bit 16-17.
	enum destination_type::type	  destination_type : 2	 = destination_type::normal;	// bit 18-19
	uint16_t					  _reserved3 : 12		 = 0;							// bit 20-31
};
STATIC_ASSERT(sizeof(interrupt_command_low_register) == 4, "ICR low must be 32 bit");

struct __attribute__((packed)) interrupt_command_high_register
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

/* =============== Command registers ==================== */
struct __attribute__((packed)) spurious_interrupt_vector_register
{
	uint8_t	 vector;		   // bits 0-7
	bool	 apic_enable : 1;  // bit 8
	uint8_t	 reserved1 : 3;	   // bits 9-11
	bool	 eoi_suppress : 1; // bit 12
	uint32_t reserved2 : 19;   // bits 13-31
};
STATIC_ASSERT(sizeof(spurious_interrupt_vector_register) == 4, "SVR must be 32-bit");

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
	return std::mmio_ptr_ro<ToType>(reinterpret_cast<volatile ToType *>(base + static_cast<uint16_t>(offset)));
}

template <typename ToType> constexpr std::mmio_ptr_wo<ToType> get_mmio_ptr_wo(uintptr_t base, lapic_registers_offset offset)
{
	return std::mmio_ptr_wo<ToType>(reinterpret_cast<volatile ToType *>(base + static_cast<uint16_t>(offset)));
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
	// Read/write registers
	static constexpr std::mmio_ptr_ro<uint32_t> lapic_id{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lapic_id)};
	static constexpr std::mmio_ptr<lapic_version_register> lapic_version{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::lapic_version)};
	static constexpr std::mmio_ptr<uint32_t> task_priority{lapic_address + static_cast<uintptr_t>(lapic_registers_offset::task_priority)};
	static constexpr std::mmio_ptr_ro<uint32_t> arbitration_priority{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::arbitration_priority)};
	static constexpr std::mmio_ptr_ro<uint32_t> process_priority{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::process_priority)};

	std::mmio_ptr_wo<uint32_t>						  remote_read;
	std::mmio_ptr<uint32_t>							  logical_destination;
	std::mmio_ptr<uint32_t>							  destination_format;
	std::mmio_ptr<spurious_interrupt_vector_register> spurious_interrupt_vector;

	std::mmio_ptr_ro<uint32_t> in_service[8];
	std::mmio_ptr_ro<uint32_t> trigger_mode[8];
	std::mmio_ptr_ro<uint32_t> interrupt_request[8];

	std::mmio_ptr_ro<uint32_t> error_status;
	std::mmio_ptr<uint32_t>	   lvt_cmci;

	static constexpr std::mmio_ptr<interrupt_command_low_register> command_low{
		lapic_address + static_cast<uintptr_t>(lapic_registers_offset::command_low)};

	static constexpr std::mmio_ptr<interrupt_command_high_register> command_high{
		get_mmio_ptr<interrupt_command_high_register>(lapic_address, lapic_registers_offset::command_high)};

	std::mmio_ptr<uint32_t> lvt_timer;
	std::mmio_ptr<uint32_t> lvt_thermal_sensor;
	std::mmio_ptr<uint32_t> lvt_performance_monitoring_counters;
	std::mmio_ptr<uint32_t> lvt_lint0;
	std::mmio_ptr<uint32_t> lvt_lint1;
	std::mmio_ptr<uint32_t> lvt_error;

	std::mmio_ptr<uint32_t>	   initial_count;
	std::mmio_ptr_ro<uint32_t> current_count;

	std::mmio_ptr<uint32_t> divide_configuration_register;

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
	std::mmio_ptr_wo<uint32_t> end_of_interrupt;
};

} // namespace apic
