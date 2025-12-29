#pragma once
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void kprintf(const char* fmt, ...);

void serial_printf(const char* prefix, uint32_t value, bool decNotHex);

#ifdef __cplusplus
}
#endif
