// kernel_cpp.cpp
#include "kernel_cpp.h"
#include "kernel_cpp.hpp"
#include "stdio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int cpp_main(void) {
	constexpr auto s = cpp_err_to_string(kernel_cpp::ErrorCode::Invalid);
	// s is a string_view pointing to static literal, no allocation
	kprintf2("Hello from C++, kernel_cpp::ErrorCode::Invalid = %s\n", s);

	return 0;
}
