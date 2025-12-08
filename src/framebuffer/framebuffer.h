#pragma once
#include "framebuffer_multiboot.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct __attribute__((packed)) color_t
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a; // a might be ignored or reserved
    };

    void do_test_c(struct framebuffer_info_t framebuffer_info);

#ifdef __cplusplus
}
#endif
