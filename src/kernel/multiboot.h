#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // void  parse_multiboot_header(uint32_t multiboot_add);
    void *get_rsdp(uint32_t mbi);

#ifdef __cplusplus
}
#endif
