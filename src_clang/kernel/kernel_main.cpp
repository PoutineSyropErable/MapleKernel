// kernel/cpp_main.cpp
#include "cpp_main.h"
import kernel.module; // C++20 module import!

// C++ entry point called from C
void cpp_kernel_main(void) {
	Module module;
	module.initialize();

	module.doSomething(42);

	int result = module.calculate(10, 20);

	module_global_function();

	// Use the module constant
	int version = MODULE_VERSION;

	// Your C++ kernel logic here
	// ...
}
