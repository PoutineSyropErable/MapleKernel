#pragma once
#include "acpi_shared.hpp"
#include <stdint.h>

namespace acpi
{

namespace madt
{

// ========== Start of madt
enum class EntryType : uint8_t
{
	processor_local_apic				  = 0,
	io_apic								  = 1,
	io_apic_interrupt_source_override	  = 2,
	io_apic_non_maskable_interrupt_source = 3,
	local_apic_non_maskable_interrupt	  = 4,
	local_apic_address_override			  = 5,
	processor_local_x2apic				  = 9,
};

extern const char *EntryTypeNames[10];

static inline const char *to_string(EntryType type)
{
	uint8_t idx = static_cast<uint8_t>(type);
	if (idx >= 10)
		return "invalid";
	return EntryTypeNames[idx];
}

struct entry_header
{
	enum EntryType entry_type;
	uint8_t		   record_length;
} __attribute__((packed));

struct MADTEntry
{
	// must cast a (MADTEntry*) -> (entry_# *)
	struct entry_header header;
	uint8_t				data[]; // actual entry data (length-2 bytes)
} __attribute__((packed));

struct MADT
{
	struct ACPISDTHeader header;
	uint32_t			 lapic_address; // for the boot processor (core 0). must be transformed to a (volatile LAPIC*)
	uint32_t			 flags;
	struct MADTEntry	 entries[];
} __attribute__((packed));

// =========== Entry 0 fields ==========
struct entry_0_flags
{
	uint32_t can_enable : 1;
	uint32_t online_capable : 1;
};
STATIC_ASSERT(sizeof(entry_0_flags) == 4, "must be a uint32_t");
struct entry_processor_local_apic
{
	entry_header  header;
	uint8_t		  acpi_processor_id;
	uint8_t		  apic_id;
	entry_0_flags flags;
} __attribute__((packed));

#define MAX_PROCESSOR_LOCAL_APIC 16
// =========== Entry 1 fields ==========
struct entry_io_apic
{
	entry_header header;
	uint8_t		 io_apic_id;
	uint8_t		 reserved;
	uint32_t	 io_apic_address;
	uint32_t	 global_system_interrupt_base;
} __attribute__((packed));
#define MAX_IO_APIC 8

// =========== Entry 2 fields ==========
struct entry_io_apic_interrupt_source_override
{
	entry_header header;
	uint8_t		 bus_source;
	uint8_t		 irq_source;
	uint32_t	 global_system_interrupt;
	uint16_t	 flags;
};
#define MAX_IO_APIC_ISO 32
// =========== Entry 3 fields ==========
struct entry_io_apic_nmi_source
{
	entry_header header;
	uint8_t		 nmi_source;
	uint8_t		 reserved;
	uint16_t	 flags;
	uint32_t	 global_system_interrupt;
};
#define MAX_IO_APIC_NMI 16
// =========== Entry 4 fields ==========
struct entry_local_apic_nmi
{
	entry_header header;
	uint8_t		 acpi_processor_id;
	uint16_t	 flags;
	bool		 LINT; // 0 or 1
};
#define MAX_LOCAL_APIC_NMI 16
// =========== Entry 5 fields ==========
struct entry_local_apic_address_override
{
	uint16_t reserved;
	uint64_t local_apic_address_64b;
};
#define MAX_LOCAL_APIC_ADDRESS_OVERRIDE 2
// =========== Entry 9 fields ==========
struct entry_processor_local_x2apic
{
	uint16_t	  reserved;
	uint32_t	  processor_local_x2apic_id;
	entry_0_flags flags;
	uint32_t	  acpi_id;
};
#define MAX_PROCESSOR_LOCAL_X2APIC 16

// ================= Parse mega function ================
struct MADTEntryCount
{
	uint32_t processor_local_apic		 = 0;
	uint32_t io_apic					 = 0;
	uint32_t io_apic_isos				 = 0;
	uint32_t io_apic_nmis				 = 0;
	uint32_t local_apic_nmis			 = 0;
	uint32_t local_apic_address_override = 0;
	uint32_t processor_local_x2apic		 = 0;
};
struct MADTParseResult
{
	// Counts
	MADTEntryCount entry_counts;

	// Entries arrays
	const entry_processor_local_apic			  *processor_local_apics[MAX_PROCESSOR_LOCAL_APIC];
	const entry_io_apic							  *io_apics[MAX_IO_APIC];
	const entry_io_apic_interrupt_source_override *io_apic_isos[MAX_IO_APIC_ISO];
	const entry_io_apic_nmi_source				  *io_apic_nmis[MAX_IO_APIC_NMI];
	const entry_local_apic_nmi					  *local_apic_nmis[MAX_LOCAL_APIC_NMI];
	const entry_local_apic_address_override		  *local_apic_address_overrides[MAX_LOCAL_APIC_ADDRESS_OVERRIDE];
	const entry_processor_local_x2apic			  *processor_local_x2apics[MAX_PROCESSOR_LOCAL_X2APIC];
};
// =========== Functions ==========
void print_madt(const MADT *madt);

struct MADTParseResult parse_madt(const MADT *madt);

void print_parsed_madt(const MADTParseResult &result);

// ========== End of madt
} // namespace madt
} // namespace acpi
