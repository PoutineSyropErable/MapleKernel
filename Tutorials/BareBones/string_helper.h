#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str);
size_t itoa(int value, char* buffer);
size_t uitoa(uint32_t value, char* buffer);
size_t int_to_hex(uint32_t value, char* buffer, int uppercase);

size_t ptr_to_hex(void* ptr, char* buffer, int uppercase);
