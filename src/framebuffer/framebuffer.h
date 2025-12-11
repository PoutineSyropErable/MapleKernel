#pragma once
#include "framebuffer_multiboot.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
    blue, green, red, reserved/transparency
    */
    struct __attribute__((packed)) color_t
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a; // a might be ignored or reserved
    };

    void do_test_c(volatile struct color_t *base_address, uint32_t width, uint32_t height, uint32_t pitch);

#ifdef __cplusplus
}
#endif
