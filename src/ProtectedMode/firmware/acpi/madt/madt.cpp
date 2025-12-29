#include "acpi.hpp"
#include "cast.h"
#include "madt.h"
#include "madt.hpp"
#include "stdio.h"
#include "string.h"

using namespace acpi::madt;

const char *acpi::madt::EntryTypeNames[10] = {
	"processor_local_apic",					 // 0
	"io_apic",								 // 1
	"io_apic_interrupt_source_override",	 // 2
	"io_apic_non_maskable_interrupt_source", // 3
	"local_apic_non_maskable_interrupt",	 // 4
	"local_apic_address_override",			 // 5
	"unused",								 // 6
	"unused",								 // 7
	"unused",								 // 8
	"processor_local_x2apic"				 // 9
};

void acpi::madt::print_madt(const acpi::madt::MADT *madt)
{
	if (!madt)
	{
		kprintf("%s MADT: NULL\n", madt);
		return;
	}
	ACPISDTHeader aligned_header = madt->header;
	print_header(&aligned_header, "MADT");

	kprintf("MADT {\n");
	kprintf("lapic address = %h\n", madt->lapic_address);
	kprintf("flags = %b\n", madt->flags);
	// kprintf("entry_type %u\n", madt->entry_type);
	// kprintf("record_length = %u\n", madt->record_length);
	kprintf("}\n");
}

MADTEntryCount get_count(const acpi::madt::MADT *madt)
{
	MADTEntryCount count{};

	uint32_t	   madt_length = madt->header.Length;
	const uint8_t *ptr		   = reinterpret_cast<const uint8_t *>(&madt->entries[0]);
	const uint8_t *end		   = reinterpret_cast<const uint8_t *>(madt) + madt_length;

	while (ptr < end)
	{
		const acpi::madt::MADTEntry *entry		   = reinterpret_cast<const acpi::madt::MADTEntry *>(ptr);
		acpi::madt::EntryType		 type		   = entry->header.entry_type;
		uint8_t						 record_length = entry->header.record_length;

		switch (type)
		{
		case acpi::madt::EntryType::processor_local_apic: count.processor_local_apic++; break;

		case acpi::madt::EntryType::io_apic: count.io_apic++; break;

		case acpi::madt::EntryType::io_apic_interrupt_source_override: count.io_apic_isos++; break;

		case acpi::madt::EntryType::io_apic_non_maskable_interrupt_source: count.io_apic_nmis++; break;

		case acpi::madt::EntryType::local_apic_non_maskable_interrupt: count.local_apic_nmis++; break;

		case acpi::madt::EntryType::local_apic_address_override: count.local_apic_address_override++; break;

		case acpi::madt::EntryType::processor_local_x2apic: count.processor_local_x2apic++; break;

		default: break; // unknown/unused entry types
		}

		ptr += record_length;
	}

	return count;
}

struct MADTParseResult acpi::madt::parse_madt(const MADT *madt)
{
	uint32_t madt_length = madt->header.Length;
	kprintf("madt length = %u\n", madt_length);
	kprintf("\n\t==========Entries===========\n\n");

	const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&madt->entries[0]);
	const uint8_t *end = reinterpret_cast<const uint8_t *>(madt) + madt_length;

	MADTEntryCount entry_write_idx;

	struct MADTParseResult ret;

	while (ptr < end)
	{
		kprintf("==========Start of entry===========\n");
		const MADTEntry *generic_entry = reinterpret_cast<const MADTEntry *>(ptr);

		EntryType type			= generic_entry->header.entry_type;
		uint8_t	  record_length = generic_entry->header.record_length;
		kprintf("record length: %u\n", record_length);

		const char *name = acpi::madt::to_string(type);
		kprintf("Name of the entry type: %s\n", name);

		switch (type)
		{
		case acpi::madt::EntryType::processor_local_apic:
		{

			const entry_processor_local_apic *entry = reinterpret_cast<const entry_processor_local_apic *>(generic_entry);
			kprintf("processor id: %u\n", entry->acpi_processor_id);
			kprintf("apic_id: %u\n", entry->apic_id);
			kprintf("flags: can_enable=%b, online_capable=%b\n", entry->flags.can_enable, entry->flags.online_capable);

			uint32_t flags_raw = BITCAST(uint32_t, entry->flags);
			kprintf("flags_raw = %u\n", flags_raw);
			ret.processor_local_apics[entry_write_idx.processor_local_apic] = entry;
			entry_write_idx.processor_local_apic++;
			break;
		}
		case acpi::madt::EntryType::io_apic:
		{

			const entry_io_apic *entry = reinterpret_cast<const entry_io_apic *>(generic_entry);
			kprintf("io apic id id: %u\n", entry->io_apic_id);
			kprintf("io apic address: %h\n", entry->io_apic_address);
			kprintf("global system interrupt base = %u\n", entry->global_system_interrupt_base);
			ret.io_apics[entry_write_idx.io_apic] = entry;
			entry_write_idx.io_apic++;
			break;
		}
		case acpi::madt::EntryType::io_apic_interrupt_source_override:
		{

			const entry_io_apic_interrupt_source_override *entry =
				reinterpret_cast<const entry_io_apic_interrupt_source_override *>(generic_entry);
			kprintf("Bus source = %u\n", entry->bus_source);
			kprintf("Irq source = %u\n", entry->irq_source);
			kprintf("global system interrupt  = %u\n", entry->global_system_interrupt);
			kprintf("flags = %u\n", entry->flags);
			ret.io_apic_isos[entry_write_idx.io_apic_isos] = entry;
			entry_write_idx.io_apic_isos++;
			break;
		}

		case acpi::madt::EntryType::io_apic_non_maskable_interrupt_source:
		{
			const entry_io_apic_nmi_source *entry = reinterpret_cast<const entry_io_apic_nmi_source *>(generic_entry);
			kprintf("nmi source = %u\n", entry->nmi_source);
			kprintf("reserved = %u\n", entry->reserved);
			kprintf("flags = %u\n", entry->flags);
			kprintf("global system interrupt = %u\n", entry->global_system_interrupt);
			ret.io_apic_nmis[entry_write_idx.io_apic_nmis] = entry;
			entry_write_idx.io_apic_nmis++;
			break;
		}

		case acpi::madt::EntryType::local_apic_non_maskable_interrupt:
		{
			const entry_local_apic_nmi *entry = reinterpret_cast<const entry_local_apic_nmi *>(generic_entry);
			kprintf("ACPI Processor ID : %u\n", entry->acpi_processor_id);
			kprintf("flags = %u\n", entry->flags);
			kprintf("LINT = %u\n", entry->LINT);
			ret.local_apic_nmis[entry_write_idx.local_apic_nmis] = entry;
			entry_write_idx.local_apic_nmis++;
			break;
		}

		case acpi::madt::EntryType::local_apic_address_override:
		{
			const entry_local_apic_address_override *entry = reinterpret_cast<const entry_local_apic_address_override *>(generic_entry);
			kprintf("reserved = %u\n", entry->reserved);
			kprintf("local apic address 64b = 0x%llx\n", entry->local_apic_address_64b);
			ret.local_apic_address_overrides[entry_write_idx.local_apic_address_override] = entry;
			entry_write_idx.local_apic_address_override++;
			break;
		}

		case acpi::madt::EntryType::processor_local_x2apic:
		{
			const entry_processor_local_x2apic *entry = reinterpret_cast<const entry_processor_local_x2apic *>(generic_entry);
			kprintf("processor_local_x2apic_id = %u\n", entry->processor_local_x2apic_id);
			kprintf("flags: can_enable=%u, online_capable=%u\n", entry->flags.can_enable, entry->flags.online_capable);
			kprintf("acpi_id = %u\n", entry->acpi_id);
			ret.processor_local_x2apics[entry_write_idx.processor_local_x2apic] = entry;
			entry_write_idx.processor_local_x2apic++;
			break;
		}

		default:
		{
			kprintf("Unknown or unused entry type\n");
			break;
		}

			// == end of while
		}

		ptr += record_length;

		kprintf("==========End of entry===========\n\n");
	}

	ret.entry_counts = entry_write_idx;
	return ret;
}

void acpi::madt::print_parsed_madt(const MADTParseResult &result)
{
	kprintf("\n==== MADT Parse Result ====\n\n");

	// Processor Local APICs
	for (uint32_t i = 0; i < result.entry_counts.processor_local_apic; ++i)
	{
		if (i == 0)
		{
			kprintf("\n");
		}
		const auto *entry = result.processor_local_apics[i];
		kprintf("Processor Local APIC %u \n", i);
		kprintf("  ACPI Processor ID: %u\n", entry->acpi_processor_id);
		kprintf("  APIC ID: %u\n", entry->apic_id);
		kprintf("  Flags: can_enable=%b, online_capable=%b\n", entry->flags.can_enable, entry->flags.online_capable);
	}
	kprintf("\n");

	// IO APICs
	for (uint32_t i = 0; i < result.entry_counts.io_apic; ++i)
	{
		if (i == 0)
			kprintf("\n");
		const auto *entry = result.io_apics[i];
		kprintf("IO APIC %u :\n", i);
		kprintf("  IO APIC ID: %u\n", entry->io_apic_id);
		kprintf("  IO APIC Address: %h\n", entry->io_apic_address);
		kprintf("  Global System Interrupt Base: %u\n", entry->global_system_interrupt_base);
	}
	kprintf("\n");

	// IO APIC Interrupt Source Overrides
	for (uint32_t i = 0; i < result.entry_counts.io_apic_isos; ++i)
	{
		if (i == 0)
			kprintf("\n");
		const auto *entry = result.io_apic_isos[i];
		kprintf("IO APIC Interrupt Source Override %u :\n", i);
		kprintf("  Bus Source: %u\n", entry->bus_source);
		kprintf("  IRQ Source: %u\n", entry->irq_source);
		kprintf("  Global System Interrupt: %u\n", entry->global_system_interrupt);
		const struct acpi::madt::io_apic_iso_flags &flags			  = entry->flags;
		char										polarities[4][20] = {"Conforms", "ActiveHigh", "Reserved", "ActiveLow"};
		char										trigger[4][20]	  = {"Conforms", "Edge", "Reserved", "Level"};
		kprintf("Flags: polarity = %s, trigger = %s\n", polarities[(uint8_t)flags.polarity], trigger[(uint8_t)flags.trigger]);
		kprintf("\n");
	}
	kprintf("\n");

	// IO APIC NMIs
	for (uint32_t i = 0; i < result.entry_counts.io_apic_nmis; ++i)
	{
		if (i == 0)
			kprintf("\n");
		const auto *entry = result.io_apic_nmis[i];
		kprintf("IO APIC NMI %u :\n", i);
		kprintf("  NMI Source: %u\n", entry->nmi_source);
		kprintf("  Flags: %u\n", entry->flags);
		kprintf("  Global System Interrupt: %u\n", entry->global_system_interrupt);
	}
	kprintf("\n");

	// Local APIC NMIs
	for (uint32_t i = 0; i < result.entry_counts.local_apic_nmis; ++i)
	{
		if (i == 0)
			kprintf("\n");
		const auto *entry = result.local_apic_nmis[i];
		kprintf("Local APIC NMI %u :\n", i);
		kprintf("  ACPI Processor ID: %u\n", entry->acpi_processor_id);
		kprintf("  Flags: %u\n", entry->flags);
		kprintf("  LINT: %u\n", entry->LINT);
	}
	kprintf("\n");

	// Local APIC Address Overrides
	for (uint32_t i = 0; i < result.entry_counts.local_apic_address_override; ++i)
	{
		if (i == 0)
			kprintf("\n");
		const auto *entry = result.local_apic_address_overrides[i];
		kprintf("Local APIC Address Override %u :\n", i);
		kprintf("  Local APIC Address 64-bit: %lx\n", entry->local_apic_address_64b);
	}
	kprintf("\n");

	// Processor Local x2APICs
	for (uint32_t i = 0; i < result.entry_counts.processor_local_x2apic; ++i)
	{
		if (i == 0)
			kprintf("\n");
		const auto *entry = result.processor_local_x2apics[i];
		kprintf("Processor Local x2APIC %u:\n", i);
		kprintf("  x2APIC ID: %u\n", entry->processor_local_x2apic_id);
		kprintf("  Flags: can_enable=%b, online_capable=%b\n", entry->flags.can_enable, entry->flags.online_capable);
		kprintf("  ACPI ID: %u\n", entry->acpi_id);
	}
	kprintf("\n");

	kprintf("\n==== End of MADT Parse Result ====\n");
}
