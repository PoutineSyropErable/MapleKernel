#!/usr/bin/env bash

set -eou pipefail
shopt -s nullglob

# ============================================
# FPU-Specific Flags (add after CFLAGS/CPPFLAGS definitions)
# ============================================

# QEMU-Compatible FPU flags for 32-bit
FPU32_FLAGS_COMMON=(
	"-m32"
	"-march=pentium4" # QEMU supports SSE2, SSE3
	"-mfpmath=sse"    # Use SSE registers instead of x87
	"-msse"           # Enable SSE
	"-msse2"          # Enable SSE2
	"-msse3"          # Enable SSE3
)

# GCC-specific FPU extras
FPU32_FLAGS_GCC=(
	"${FPU32_FLAGS_COMMON[@]}"
	"-mtune=generic" # GCC-specific tuning
	"-mno-avx"       # Disable AVX (QEMU doesn't have it)
	"-mno-avx2"      # Disable AVX2
	"-mno-avx512f"   # Disable AVX512
)

# Clang-specific FPU extras
FPU32_FLAGS_CLANG=(
	"${FPU32_FLAGS_COMMON[@]}"
	"-mstackrealign" # Clang needs this for 32-bit
	"-mno-avx"       # Disable AVX
	"-mno-avx2"      # Disable AVX2
	"-mno-avx512f"   # Disable AVX512
)

#------------- OLDER VERSION---------------
exit 0
# ============================================
# GCC 32-bit FPU Flags
# ============================================
FPU32_FLAGS_GNU=(
	# Architecture and mode
	"-m32"            # Compile for 32-bit
	"-march=pentium4" # Baseline with SSE2

	# FPU math model (CHOOSE ONE)
	"-mfpmath=sse" # Use SSE registers for FP (modern)
	# OR for x87-only:
	# "-mfpmath=387"        # Use x87 FPU stack (legacy)

	# SSE support
	"-msse"   # Enable SSE instructions
	"-msse2"  # Enable SSE2 instructions
	"-msse3"  # Enable SSE3 instructions
	"-mssse3" # Enable SSSE3 instructions

	# Optional advanced features (if CPU supports)
	"-msse4.1" # SSE4.1
	"-msse4.2" # SSE4.2
	"-mavx"    # AVX
	"-mfma"    # FMA
	"-mavx2"   # AVX2

	# Optimization
	"-O2"         # Good optimization level
	"-ffast-math" # Aggressive FP optimizations (careful!)

	# For kernel development
	"-ffreestanding"       # No standard library
	"-nostdlib"            # Don't link standard libs
	"-fno-builtin"         # Don't use builtin functions
	"-fno-stack-protector" # No stack protection
	"-fno-pic"             # No position-independent code
	"-mno-red-zone"        # No red zone (for x86_64, but harmless for 32-bit)
)

# ============================================
# Clang 32-bit FPU Flags
# ============================================
FPU32_FLAGS_CLANG=(
	# Architecture and mode
	"-m32"            # Compile for 32-bit
	"-march=pentium4" # Baseline with SSE2

	# FPU math model (CHOOSE ONE)
	"-mfpmath=sse" # Use SSE registers for FP
	# OR:
	# "-mfpmath=387"        # Use x87 FPU stack

	# SSE support
	"-msse"   # Enable SSE
	"-msse2"  # Enable SSE2
	"-msse3"  # Enable SSE3
	"-mssse3" # Enable SSSE3

	# Optional advanced features
	"-msse4.1" # SSE4.1
	"-msse4.2" # SSE4.2
	"-mavx"    # AVX
	"-mfma"    # FMA
	"-mavx2"   # AVX2

	# Optimization
	"-O2"         # Optimization level 2
	"-ffast-math" # Fast math (careful!)

	# For kernel development
	"-ffreestanding"       # Freestanding environment
	"-nostdlib"            # No standard library
	"-fno-builtin"         # No builtins
	"-fno-stack-protector" # No stack protection
	"-fno-pic"             # No position-independent code
	"-mstackrealign"       # Force stack alignment (Clang specific)
)

# ============================================
# QEMU-Specific Safe Flags (for basic emulation)
# ============================================

FPU32_FLAGS_QEMU_SAFE=(
	"-march=pentium" # Older CPU, no SSE
	"-mfpmath=387"   # Use x87 only
	"-mno-sse"       # Explicitly disable SSE
	"-mno-sse2"      # Disable SSE2
	"-mno-sse3"      # Disable SSE3
)
