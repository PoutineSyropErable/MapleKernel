#include "acpi.hpp"
#include "stdio.h"
#include "string.h"

using namespace acpi;

bool doChecksum(ACPISDTHeader *tableHeader)
{
	unsigned char sum = 0;

	for (size_t i = 0; i < tableHeader->Length; i++)
	{
		sum += ((char *)tableHeader)[i];
	}

	return sum == 0;
}

void acpi::print_rsdp(const RSDP *rsdp)
{
	if (!rsdp)
	{
		kprintf("RSDP: NULL\n");
		return;
	}

	kprintf("RSDP {\n");
	kprintf("  Signature: \"%s;8\"\n", rsdp->Signature);
	kprintf("  Checksum: %u\n", rsdp->Checksum);
	kprintf("  OEMID: \"%s;6\"\n", rsdp->OEMID);
	kprintf("  Revision: %u\n", rsdp->Revision);
	kprintf("  RsdtAddress: %h\n", rsdp->RsdtAddress);
	kprintf("}\n");
}

void acpi::print_header(const ACPISDTHeader *header, const char *table_name)
{
	if (!header)
	{
		kprintf("%s Header: NULL\n", table_name);
		return;
	}

	kprintf("%s Header {\n", table_name);
	kprintf("  Signature: \"%s;4\"\n", header->Signature);
	kprintf("  Length: %u bytes\n", header->Length);
	kprintf("  Revision: %u\n", header->Revision);
	kprintf("  Checksum: %u\n", header->Checksum);
	kprintf("  OEMID: \"%s;6\"\n", header->OEMID);
	kprintf("  OEMTableID: \"%s;8\"\n", header->OEMTableID);
	kprintf("  OEMRevision: %u\n", header->OEMRevision);
	kprintf("  CreatorID: %u\n", header->CreatorID);
	kprintf("  CreatorRevision: %u\n", header->CreatorRevision);
	kprintf("}\n");
}

void acpi::print_madt(const MADT *madt)
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

void acpi::print_rsdt(const RSDT *rsdt)
{

	if (!rsdt)
	{
		kprintf("RSDT: NULL\n");
		return;
	}

	// Copy header to ensure alignment
	ACPISDTHeader aligned_header = rsdt->header;
	return acpi::print_header(&aligned_header, "RSDT");

	// Print the entries?
}

void *findFACP(RSDT *RootSDT)
{
	RSDT *rsdt	  = (RSDT *)RootSDT;
	int	  entries = (rsdt->header.Length - sizeof(rsdt->header)) / 4;

	for (int i = 0; i < entries; i++)
	{
		ACPISDTHeader *h = (ACPISDTHeader *)rsdt->Entries[i];
		if (!strncmp(h->Signature, "FACP", 4))
			return (void *)h;
	}

	// No FACP found
	return NULL;
}

MADT *acpi::findMADT(RSDT *rsdt)
{
	int entries = (rsdt->header.Length - sizeof(rsdt->header)) / 4;

	for (int i = 0; i < entries; i++)
	{
		ACPISDTHeader *h = (ACPISDTHeader *)rsdt->Entries[i];
		if (!strncmp(h->Signature, "APIC", 4))
		{
			// print_header(h, "MADT?\n");
			return (MADT *)h;
		}
	}

	// No FACP found
	return NULL;
}
