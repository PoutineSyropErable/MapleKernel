#pragma once
#include "acpi_shared.hpp"
#include "madt.hpp"
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace acpi
{

struct RSDT
{
	struct ACPISDTHeader header;

	// Array of 32-bit physical addresses pointing to other ACPI tables
	uint32_t Entries[];
	// uint32_t entires_c = (header.Length - sizeof(header)) / 4;
} __attribute__((packed));

struct RSDP
{
	char		 Signature[8];
	uint8_t		 Checksum;
	char		 OEMID[6];
	uint8_t		 Revision;
	struct RSDT *RsdtAddress; // 32 bit only

} __attribute__((packed));

STATIC_ASSERT(sizeof(RSDP) == 20, "RSDT_t must be correct size. If it's 24, then it's a 64 bit ptr\n");

struct XSDP_t
{
	char	 Signature[8];
	uint8_t	 Checksum;
	char	 OEMID[6];
	uint8_t	 Revision;
	uint32_t RsdtAddress; // deprecated since version 2.0

	uint32_t Length;
	uint64_t XsdtAddress;
	uint8_t	 ExtendedChecksum;
	uint8_t	 reserved[3];
} __attribute__((packed));

// ================ Start of MADT ===============
madt::MADT *findMADT(RSDT *rsdt);

// ============== END of MADT

void print_rsdp(const RSDP *rsdp);
void print_header(const ACPISDTHeader *header, const char *table_name);
void print_rsdt(const RSDT *rsdt);

} // namespace acpi
