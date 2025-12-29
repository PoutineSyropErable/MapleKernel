#pragma once
#include "intrinsics.h"

const uint32_t unused_port = 0x80;

static inline void io_wait(void)
{
    __outb(unused_port, 0);
}
