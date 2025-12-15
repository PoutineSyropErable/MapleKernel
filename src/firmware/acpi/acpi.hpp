#pragma once
#include <stddef.h>
#include <stdint.h>

namespace acpi
{

// struct XSDP_t;
//
// union XSDPT_uts
// {
//     struct XSDP_t *ptr;
//     uint32_t       raw;
// };

struct RSDP_t
{
	char	 Signature[8];
	uint8_t	 Checksum;
	char	 OEMID[6];
	uint8_t	 Revision;
	uint32_t RsdtAddress;
} __attribute__((packed));

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

} // namespace acpi
