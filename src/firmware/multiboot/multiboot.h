#pragma once
#include "framebuffer_multiboot.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum rsdp_type_c
    {
        RSDP_TYPE_NULL   = 0,
        RSDP_TYPE_V1_OLD = 1,
        RSDP_TYPE_V2_New = 2,
    };

    struct rsdp_tagged_c
    {
        enum rsdp_type_c new_or_old;
        void            *rsdp;
    };

    // void  parse_multiboot_header(uint32_t multiboot_add);
    struct rsdp_tagged_c get_rsdp(uint32_t mbi);

    bool validate_rsdp_c(void *rsdp_ptr);

    struct framebuffer_info_t get_framebuffer(uint32_t mid_addr);

#ifdef __cplusplus
}
#endif
