#pragma once
#include <stdint.h>

namespace kernel_cpp
{

enum class ErrorCode : uint8_t
{
	None	= 0,
	Invalid = 1,
	Timeout = 2,
};

// Freestanding, constexpr string mapping
constexpr const char *cpp_err_to_string(ErrorCode e)
{
	switch (e)
	{
	case ErrorCode::None: return "None";
	case ErrorCode::Invalid: return "Invalid";
	case ErrorCode::Timeout: return "Timeout";
	}
	return "Unknown";
}

} // namespace kernel_cpp
