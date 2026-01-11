#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus 
extern "C" {
#endif

size_t strlen(const char* str);
size_t itoa(int value, char* buffer);
size_t uitoa(uint32_t value, char* buffer);
size_t ftoa(float value, char* buffer, uint32_t precision);

size_t uint_to_hex(uint32_t value, char* buffer, int uppercase);
size_t ptr_to_hex(void* ptr, char* buffer, int uppercase);

#define CHAR_TO_UINT_INVALID 11

uint32_t string_to_uint(const char* str);


#ifdef __cplusplus 
}
#endif
