#pragma once
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace acpi
{

struct ACPISDTHeader
{
	char	 Signature[4];
	uint32_t Length;
	uint8_t	 Revision;
	uint8_t	 Checksum;
	char	 OEMID[6];
	char	 OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
};

} // namespace acpi
