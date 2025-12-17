#include "assert.h"
#include "cpuid.h"
#include "cpuid.hpp"

using namespace cpuid;

cpuid_error validate_cpuid_function(CpuidFunction cpu_function) noexcept
{
	uint32_t raw = static_cast<uint32_t>(cpu_function);

	// Basic functions: 0x0…max_basic
	if (raw <= static_cast<uint32_t>(CpuidFunction::XSAVE_Features))
	{
		uint32_t max_basic = get_cpuid_max();
		if (raw > max_basic)
			return cpuid_error::cpuid_function_not_supported_by_hardware;
		return cpuid_error::none;
	}

	// Extended functions: 0x80000000…max_extended
	if (raw >= static_cast<uint32_t>(CpuidFunction::Extended_MaxFunction))
	{
		uint32_t max_ext = get_cpuid_max_extended();
		if (raw > max_ext)
			return cpuid_error::cpuid_extended_function_not_supported_by_hardware;
		return cpuid_error::none;
	}

	// Anything else is unknown
	return cpuid_error::unknown_cpu_function;
}

struct cpuid_verified_result cpuid::call_cpuid(enum CpuidFunction cpu_function, union Subleaf_ECX subleaf_ecx)
{
	struct cpuid_verified_result ret;
	ret.regs			 = {.eax = 0, .ebx = 0, .ecx = 0, .edx = 0};
	uint32_t raw_subleaf = 0; // init here, because of a compiler warning?

	if (cpu_function != CpuidFunction::Basic_MaxFunction && cpu_function != CpuidFunction::Extended_MaxFunction)
	{

		cpuid_error err = validate_cpuid_function(cpu_function);
		if (err != cpuid_error::none)
		{
			ret.err = err;
			return ret;
		}
	}

	switch (cpu_function)
	{
	case CpuidFunction::ExtendedFeatures: // 0x7
		raw_subleaf = static_cast<uint32_t>(subleaf_ecx.extended_features_subleaf);
		break;

	case CpuidFunction::ExtendedTopology: // 0xB
		raw_subleaf = static_cast<uint32_t>(subleaf_ecx.extended_topology_subleaf);
		break;

	case CpuidFunction::XSAVE_Features: // 0xD
		raw_subleaf = static_cast<uint32_t>(subleaf_ecx.extended_save_feature_subleaf);
		break;

	// For functions without subleafs
	case CpuidFunction::Basic_MaxFunction:
	case CpuidFunction::ProcessorInfo:
	case CpuidFunction::Cache_TLB:
	case CpuidFunction::SerialNumber:
	case CpuidFunction::DeterministicCache:
	case CpuidFunction::MonitorMwait:
	case CpuidFunction::ThermalPower:
	case CpuidFunction::DirectCacheAccess:
	case CpuidFunction::PerformanceMonitoring:
	//
	case CpuidFunction::Extended_MaxFunction:
	case CpuidFunction::Extended_Features:
	case CpuidFunction::Brand_String1:
	case CpuidFunction::Brand_String2:
	case CpuidFunction::Brand_String3:
	case CpuidFunction::L2L3_CacheInfo:
	case CpuidFunction::AdvancedPowerMgmt:
	case CpuidFunction::PhysicalAddressInfo:
		if (subleaf_ecx.raw != 0)
		{
			ret.err = cpuid_error::function_doesnt_have_subleaf;
			// ecx should be ignored, but whoever calls this function
			// should know that, so let's return an error.
			return ret;
		}
		break;

	default:
		ret.err = cpuid_error::unknown_cpu_function;
		//
		return ret;
	}

	ret.err	 = cpuid_error::none;
	ret.regs = _cpuid((uint32_t)cpu_function, raw_subleaf);
	return ret;
}

// ================================= MAX BASIC ===============================
uint32_t max_cpuid_command = 0;
char	 cpuid_vendor[13]  = {0};

const char *get_cpuid_vendor()
{
	if (*cpuid_vendor)
	{
		return cpuid_vendor;
	}

	cpuid_verified_result result = call_cpuid(CpuidFunction::Basic_MaxFunction, Subleaf_ECX{.raw = 0});
#ifdef DEBUG
	if (result.has_error())
	{
		abort_msg("This is impossible case, result has an error: %u\n", result.err);
	}
#endif

	max_cpuid_command = result.regs.eax;
	// CPUID.0 vendor string order: EBX, EDX, ECX
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[0]) = result.regs.ebx;
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[4]) = result.regs.edx;
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[8]) = result.regs.ecx;

	cpuid_vendor[12] = '\0';
	return cpuid_vendor;
}

uint32_t get_cpuid_max()
{
	if (max_cpuid_command != 0)
	{
		return max_cpuid_command;
	}

	cpuid_verified_result result = call_cpuid(CpuidFunction::Extended_MaxFunction, Subleaf_ECX{.raw = 0});
#ifdef DEBUG
	if (result.has_error())
	{
		abort_msg("CPUID extended max failed: %u\n", result.err);
	}
#endif

	max_cpuid_command								= result.regs.eax;
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[0]) = result.regs.ebx;
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[4]) = result.regs.edx;
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[8]) = result.regs.ecx;
	return result.regs.eax;
}

struct cpuid_max_basic_feature_and_vendor get_cpuid_max_and_vendor()
{
	struct cpuid_max_basic_feature_and_vendor ret;

	cpuid_verified_result result = call_cpuid(CpuidFunction::Basic_MaxFunction, Subleaf_ECX{.raw = 0});
#ifdef DEBUG
	if (result.has_error())
	{
		abort_msg("This is impossible case, result has an error: %u\n", result.err);
	}
#endif

	max_cpuid_command = result.regs.eax;
	// CPUID.0 vendor string order: EBX, EDX, ECX
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[0]) = result.regs.ebx;
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[4]) = result.regs.edx;
	*reinterpret_cast<uint32_t *>(&cpuid_vendor[8]) = result.regs.ecx;
	cpuid_vendor[12]								= '\0';

	ret.regs   = result.regs;
	ret.vendor = cpuid_vendor;

	return ret;
}

// ================== Max Extended
uint32_t get_cpuid_max_extended()
{
	static uint32_t max_extended_cpuid_command = 0;
	if (max_extended_cpuid_command != 0)
	{
		return max_extended_cpuid_command;
	}

	cpuid_verified_result result = call_cpuid(CpuidFunction::Extended_MaxFunction, Subleaf_ECX{.raw = 0});
#ifdef DEBUG
	if (result.has_error())
	{
		abort_msg("CPUID extended max failed: %u\n", result.err);
	}
#endif

	// CPUID.80000000:EAX = max extended leaf
	max_extended_cpuid_command = result.regs.eax;
	return result.regs.eax;
}

// ======================= Brand string

char		brand_string[49] = {0}; // 48 chars + null terminator
const char *get_cpuid_brand_string()
{

	// static char brand_string[49] = {0}; // 48 chars + null terminator
	if (brand_string[0] != '\0') // already cached
		return brand_string;

	CpuidFunction leaves[3] = {CpuidFunction::Brand_String1, CpuidFunction::Brand_String2, CpuidFunction::Brand_String3};

	uint32_t *ptr = reinterpret_cast<uint32_t *>(brand_string);

	for (size_t i = 0; i < 3; ++i)
	{
		cpuid_verified_result res = call_cpuid(leaves[i], Subleaf_ECX{.raw = 0});
#ifndef DEBUG
		if (res.has_error())
			abort_msg("CPUID brand string leaf failed: %u, %s\n", res.err, to_string(res.err));
#endif
		*ptr++ = res.regs.eax;
		*ptr++ = res.regs.ebx;
		*ptr++ = res.regs.ecx;
		*ptr++ = res.regs.edx;
	}

	brand_string[48] = '\0'; // ensure null termination
	return brand_string;
}

const char *cpuid_error_to_string(uint32_t err_uint)
{
	return to_string((enum cpuid_error)err_uint);
}
