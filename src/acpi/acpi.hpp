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
    char     Signature[8];
    uint8_t  Checksum;
    char     OEMID[6];
    uint8_t  Revision;
    uint32_t RsdtAddress;
} __attribute__((packed));

} // namespace acpi
