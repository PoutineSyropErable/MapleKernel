#pragma once
#include <stdint.h>
// Arguments: a = eax, b = ebx
// Returns: 16-bit result in EAX
extern uint16_t call_add16(uint16_t a, uint16_t b);
