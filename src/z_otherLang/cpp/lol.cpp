#include "stdio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum class ErrorCode : uint8_t {
	None = 0,
	Invalid = 1,
	Timeout = 2,
};

constexpr const char* to_string(ErrorCode e) {
	switch (e) {
	case ErrorCode::None:
		return "None";
	case ErrorCode::Invalid:
		return "Invalid";
	case ErrorCode::Timeout:
		return "Timeout";
	}
	return "Unknown";
}

int cpp_main() {
	constexpr auto s = to_string(ErrorCode::Invalid);
	// s is a string_view pointing to static literal, no allocation
	kprintf2("%s\n", s);

	return 0;
}
