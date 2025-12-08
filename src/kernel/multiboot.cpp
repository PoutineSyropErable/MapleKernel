// multiboot2.cpp - C++ implementation
#include "assert.h"
#include "multiboot.hpp"
#include "stdio.h"
#include <stddef.h>

namespace multiboot
{

// Check if RSDP has valid signature
static bool check_rsdp_signature(const void *rsdp_ptr)
{
    const char *sig = static_cast<const char *>(rsdp_ptr);

    // Check for "RSD PTR " signature
    return (sig[0] == 'R' && sig[1] == 'S' && sig[2] == 'D' && sig[3] == ' ' && sig[4] == 'P' && sig[5] == 'T' && sig[6] == 'R' &&
            sig[7] == ' ');
}

// Calculate RSDP checksum
static bool check_rsdp_checksum(const void *rsdp_ptr)
{
    const uint8_t *data = static_cast<const uint8_t *>(rsdp_ptr);

    // First, check if it's ACPI 1.0 (20 bytes) or 2.0+ (36 bytes)
    // Revision is at byte 15 (0-based)
    uint8_t revision = data[15];
    size_t  length   = (revision == 0) ? 20 : 36;

    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++)
    {
        sum += data[i];
    }

    return (sum & 0xFF) == 0;
}

// Main implementation
void *find_rsdp(info *mbi)
{
    if (!mbi)
    {
        // abort_msg("null mbi");
        abort_msg("null mbi\n");
        return nullptr;
    }

    // Check if MBI size is reasonable
    if (mbi->total_size < 8 + 8)
    { // header + minimum tag
        abort_msg("too small\n");
        abort();
        return nullptr;
    }

    tag      *current  = tag_first(mbi);
    uintptr_t end_addr = reinterpret_cast<uintptr_t>(mbi) + mbi->total_size;

    while (reinterpret_cast<uintptr_t>(current) < end_addr)
    {

        kprintf("Current type: %h\n", current->type);
        kprintf("Current size: %h\n", current->size);
        // Check for end tag
        if (current->type == END && current->size == 8)
        {
            break;
        }

        // Check for ACPI RSDP tags
        if (current->type == ACPI_OLD || current->type == ACPI_NEW)
        {
            // RSDP data starts right after the tag header
            void *rsdp_ptr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(current) + sizeof(tag));

            // Quick validation
            if (check_rsdp_signature(rsdp_ptr) && check_rsdp_checksum(rsdp_ptr))
            {
                return rsdp_ptr;
            }
        }

        current = tag_next(current);
    }

    abort_msg("Nothing found!\n");
    return nullptr;
}

bool validate_rsdp(void *rsdp_ptr)
{
    if (!rsdp_ptr)
    {
        return false;
    }

    return check_rsdp_signature(rsdp_ptr) && check_rsdp_checksum(rsdp_ptr);
}

} // namespace multiboot

// C interface implementation
extern "C"
{

    void *get_rsdp(uint32_t mbi_addr)
    {
        return multiboot::find_rsdp(reinterpret_cast<multiboot::info *>(mbi_addr));
    }

    int multiboot_validate_rsdp(void *rsdp)
    {
        return multiboot::validate_rsdp(rsdp) ? 1 : 0;
    }
}
