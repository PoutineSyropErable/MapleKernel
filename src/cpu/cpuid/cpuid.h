#pragma once
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

	uint32_t cpuid_supported_check();
	struct cpuid_reg
	{
		uint32_t eax;
		uint32_t ebx;
		uint32_t ecx;
		uint32_t edx;
	};

	struct cpuid_reg _cpuid(uint32_t cpuid_function, uint32_t subleaf);

	struct cpuid_max_basic_feature_and_vendor
	{
		struct cpuid_reg regs;
		char			*vendor;
	};
	const char *get_cpuid_vendor();
	uint32_t	get_cpuid_max();
	uint32_t	get_cpuid_max_extended();

	const char *get_cpuid_brand_string();
	const char *cpuid_err_to_string();

#ifdef __cplusplus
}
#endif
