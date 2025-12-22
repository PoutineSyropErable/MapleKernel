#pragma once
#include <stdint.h>

#define MAX_CORE_COUNT 8

extern "C" uint8_t last_interrupt_recieved[MAX_CORE_COUNT];

#define INTERRUPT_ENTERED_MAIN 41
