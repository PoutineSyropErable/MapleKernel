// multiboot2.cpp - C++ implementation
#include "assert.h"
#include "multiboot.h"
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
struct multiboot::rsdp_tagged_t find_rsdp(info *mbi)
{

    struct multiboot::rsdp_tagged_t ret;
    if (!mbi)
    {
        abort_msg("null mbi\n");
    }

    // Check if MBI size is reasonable
    if (mbi->total_size < 8 + 8)
    { // header + minimum tag
        abort_msg("too small\n");
        abort();
    }

    tag      *current  = tag_first(mbi);
    uintptr_t end_addr = reinterpret_cast<uintptr_t>(mbi) + mbi->total_size;

    while (reinterpret_cast<uintptr_t>(current) < end_addr)
    {

        // kprintf("Current type: %s\n", multiboot::type_to_str(current->type));
        // kprintf("Current size: %h\n", current->size);
        // Check for end tag
        if (current->type == multiboot::tag_type::END && current->size == 8)
        {
            break;
        }

        // Check for ACPI RSDP tags
        if (current->type == multiboot::tag_type::ACPI_NEW)
        {
            // RSDP data starts right after the tag header
            void *rsdp_ptr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(current) + sizeof(tag));
            kprintf("NEW\n");

            // Quick validation
            if (check_rsdp_signature(rsdp_ptr) && check_rsdp_checksum(rsdp_ptr))
            {
                ret.rsdp       = rsdp_ptr;
                ret.new_or_old = multiboot::rsdp_type::NEW_V2;
                return ret;
            }
        }

        // Check for ACPI RSDP tags
        else if (current->type == multiboot::tag_type::ACPI_OLD)
        {
            // RSDP data starts right after the tag header
            void *rsdp_ptr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(current) + sizeof(tag));
            kprintf("OLD\n");

            // Quick validation
            if (check_rsdp_signature(rsdp_ptr) && check_rsdp_checksum(rsdp_ptr))
            {
                ret.rsdp       = rsdp_ptr;
                ret.new_or_old = multiboot::rsdp_type::OLD_V1;
                return ret;
            }
        }

        current = tag_next(current);
    }

    abort_msg("Nothing found!\n");
}

struct framebuffer_info_t find_framebuffer(info *mbi)
{

    struct multiboot::rsdp_tagged_t ret;
    if (!mbi)
    {
        abort_msg("null mbi\n");
    }

    // Check if MBI size is reasonable
    if (mbi->total_size < 8 + 8)
    { // header + minimum tag
        abort_msg("too small\n");
        abort();
    }

    tag      *current  = tag_first(mbi);
    uintptr_t end_addr = reinterpret_cast<uintptr_t>(mbi) + mbi->total_size;

    while (reinterpret_cast<uintptr_t>(current) < end_addr)
    {

        // Check for end tag
        if (current->type == multiboot::tag_type::END && current->size == 8)
        {
            break;
        }

        // Check for ACPI RSDP tags
        if (current->type == multiboot::tag_type::FRAMEBUFFER)
        {

            framebuffer_info_t *ret_ptr = reinterpret_cast<framebuffer_info_t *>(current);
            framebuffer_info_t  ret     = *ret_ptr;

            return ret;
        }

        current = tag_next(current);
    }

    abort_msg("Nothing found!\n");
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

    struct rsdp_tagged_c get_rsdp(uint32_t mbi_addr)
    {
        struct multiboot::rsdp_tagged_t t = multiboot::find_rsdp(reinterpret_cast<multiboot::info *>(mbi_addr));

        struct rsdp_tagged_c ret;
        ret.new_or_old = (enum rsdp_type_c)(int)t.new_or_old;
        ret.rsdp       = t.rsdp;
        return ret;
    }

    int multiboot_validate_rsdp(void *rsdp)
    {
        return multiboot::validate_rsdp(rsdp) ? 1 : 0;
    }

    bool validate_rsdp_c(void *rsdp_ptr)
    {
        return multiboot_validate_rsdp(rsdp_ptr);
    }

    struct framebuffer_info_t get_framebuffer(uint32_t mid_addr)
    {
        multiboot::info *mbi = (multiboot::info *)mid_addr;
        return multiboot::find_framebuffer(mbi);
    }
}
