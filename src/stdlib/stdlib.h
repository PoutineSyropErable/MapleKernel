#pragma once
#include <stddef.h>
#include <stdint.h>

static inline void *alloca(size_t size)
{
	return __builtin_alloca(size);
}
