#pragma once

// Macro version of bit_cast
#define BITCAST(ToType, FromValue)                                                                                                         \
	({                                                                                                                                     \
		ToType _to;                                                                                                                        \
		__builtin_memcpy(&_to, &(FromValue), sizeof(ToType));                                                                              \
		_to;                                                                                                                               \
	})
