#pragma once
#include "cpuid.h"
#include <stddef.h>
#include <stdint.h>

namespace cpuid
{

enum class CpuidFunction : uint32_t
{
	// Basic functions
	Basic_MaxFunction	  = 0x0, // Returns maximum supported basic function + vendor ID string
	ProcessorInfo		  = 0x1, // Processor version info and feature flags
	Cache_TLB			  = 0x2, // Legacy Cache and TLB descriptors
	SerialNumber		  = 0x3, // Processor serial number (obsolete)
	DeterministicCache	  = 0x4, // Deterministic Cache Parameters (L1, L2, L3)
	MonitorMwait		  = 0x5, // MONITOR/MWAIT info
	ThermalPower		  = 0x6, // Thermal and power management info
	ExtendedFeatures	  = 0x7, // Extended feature flags (subleaf in ECX)
	DirectCacheAccess	  = 0x9, // Direct cache access / SMM info
	PerformanceMonitoring = 0xA, // Architectural performance monitoring
	ExtendedTopology	  = 0xB, // Extended processor topology (subleaf in ECX)
	XSAVE_Features		  = 0xD, // XSAVE feature flags (subleaf in ECX)

	// Extended functions (0x80000000+)
	Extended_MaxFunction = 0x80000000, // Returns max extended function supported
	Extended_Features	 = 0x80000001, // Extended feature flags (NX, etc.)
	Brand_String1		 = 0x80000002, // Processor brand string part 1
	Brand_String2		 = 0x80000003, // Processor brand string part 2
	Brand_String3		 = 0x80000004, // Processor brand string part 3
	L2L3_CacheInfo		 = 0x80000006, // L2/L3 cache info
	AdvancedPowerMgmt	 = 0x80000007, // Advanced power management info
	PhysicalAddressInfo	 = 0x80000008, // Physical/virtual address size info
};

// Subleafs for CPUID function 0x7: Extended Features
enum class Subleaf7 : uint32_t
{
	MainLeaf = 0, // EAX=7, ECX=0: EBX/ECX/EDX feature flags
	Leaf1	 = 1, // EAX=7, ECX=1: Some processors expose more flags
	Leaf2	 = 2,
	// Add more if necessary, up to CPU-reported max
};

// Subleafs for CPUID function 0xB: Extended Topology Enumeration
enum class SubleafB : uint32_t
{
	SMT		= 0, // Thread level topology
	Core	= 1, // Core level topology
	Package = 2, // Package level
				 // Additional subleafs can be defined based on CPUID max subleaf
};

// Subleafs for CPUID function 0xD: XSAVE/XFEATURE info
enum class SubleafD : uint32_t
{
	XSaveFeatureBits = 0, // Standard feature flags
	XSAVE_YMM		 = 1, // AVX state
	XSAVE_ZMM_Hi256	 = 2, // AVX-512 upper 256-bit
	XSAVE_OPMASK	 = 3, // AVX-512 opmask registers
	XSAVE_ZMM_Hi16	 = 4, // AVX-512 upper 16 ZMM registers
						  // Extend as needed
};

union Subleaf_ECX
{
	enum Subleaf7 extended_features_subleaf;
	enum SubleafB extended_topology_subleaf;
	enum SubleafD extended_save_feature_subleaf;
	uint32_t	  raw;
};

// Maybe remove this.
constexpr struct CPUID_BORDERS
{
	uint32_t bad_bigger_then1  = (uint32_t)CpuidFunction::XSAVE_Features;
	uint32_t bad_smaller_then1 = (uint32_t)CpuidFunction::Extended_MaxFunction;

	uint32_t bad_bigger_then2 = (uint32_t)CpuidFunction::PhysicalAddressInfo;

} cpuid_borders;

enum class cpuid_error : int
{
	none = 0,
	unknown_cpu_function,
	function_doesnt_have_subleaf,
	cpuid_function_not_supported_by_hardware,
	cpuid_extended_function_not_supported_by_hardware,
};

constexpr const char *to_string(cpuid_error err) noexcept
{
	// Array of string literals matching the enum order
	constexpr const char *names[] = {"none", "unknown_cpu_function", "function_doesnt_have_subleaf",
		"cpuid_function_not_supported_by_hardware", "cpuid_extennded_function_not_supported_by_hardware"};

	int index = static_cast<int>(err);
	// Bounds check
	if (index < 0 || index >= static_cast<int>(sizeof(names) / sizeof(names[0])))
		return "invalid_cpuid_error";

	return names[index];
}

struct cpuid_verified_result
{
	struct cpuid_reg regs;
	enum cpuid_error err;

	constexpr bool has_error() noexcept
	{
		return err != cpuid_error::none;
	}
};

struct cpuid_verified_result call_cpuid(enum CpuidFunction cpu_function, union Subleaf_ECX subleaf_ecx);

} // namespace cpuid
