// kernel_cpp.h
#pragma once
#include "multiboot.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	// C-visible entry point to transfer control to C++ code
	int cpp_event_loop(void);

	struct cpp_main_args
	{
		void				*rsdp_v;
		struct entry_point_c kernel64_address_information;
	};
	int cpp_main(struct cpp_main_args args);

#ifdef __cplusplus
}
#endif
