#pragma once
#include <stdint.h>

#define MAX_CORE_COUNT 8
#define NEW_GDT_ENTRY_COUNT (4 + 2 * MAX_CORE_COUNT)

extern "C" uint8_t last_interrupt_recieved[MAX_CORE_COUNT];

#define INTERRUPT_ENTERED_MAIN 41
