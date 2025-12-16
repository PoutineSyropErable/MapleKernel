#!/usr/bin/env bash
#
# ==================================================
# BUILD SCRIPT — C++23 MODULES (SINGLE-STEP)
# ==================================================
#
# Core invariants:
#   -fmodule-output creates .o and .pcm in one step
#   .pcm files are created with same basename as .o files
#   Module implementations find .pcm via -fprebuilt-module-path
#

# --------------------------------------------------
# Output directory
# --------------------------------------------------

set -eou pipefail

cd "$(dirname "$0")"

BUILD="./build"
# BUILD="../build"
mkdir -p "$BUILD"

MOD1="./mod1/"
STDIO_INC="../stdio"
STDLIB_INC="../stdlib"

# ==================================================
# COMMON FLAGS FOR ALL TARGETS
# ==================================================
# Base flags for i686-elf bare-metal kernel
# No standard library, no host dependencies

COMMON_FLAGS=(
	-target i686-elf     # Target architecture
	-ffreestanding       # No standard library
	-nostdlib            # Don't link with standard libraries
	-nodefaultlibs       # Don't use default libraries
	-fno-builtin         # Don't use builtin functions
	-fno-exceptions      # No C++ exceptions (bare metal)
	-fno-rtti            # No RTTI (bare metal)
	-fno-stack-protector # No stack protection
	-m32                 # 32-bit mode
	-march=i686          # i686 architecture
	-mno-red-zone        # No red zone (SysV ABI)
	-mno-mmx             # Disable MMX
	-mno-sse             # Disable SSE
	-mno-sse2            # Disable SSE2
	-mno-80387           # Disable FPU (optional)
	-I"$STDIO_INC"       # Custom stdio headers
	-I"$STDLIB_INC"      # Custom stdlib headers
)

# ==================================================
# C-SPECIFIC FLAGS
# ==================================================

CFLAGS=(
	"${COMMON_FLAGS[@]}" # Include all common flags
	-std=c23             # C23 standard
	-Wall                # All warnings
	-Wextra              # Extra warnings
)

# ==================================================
# C++-SPECIFIC FLAGS
# ==================================================
# Additional flags for C++ module compilation

CXXFLAGS=(
	"${COMMON_FLAGS[@]}"            # Include all common flags
	-std=c++23                      # C++23 with modules
	-Wall                           # All warnings
	-Wextra                         # Extra warnings
	-fprebuilt-module-path="$BUILD" # Where to find compiled .pcm files
)

# ==================================================
# PHASE 1 — MODULE INTERFACES → .o + .pcm
# ==================================================
#
# Single-step compilation generates both artifacts.
# .o contains object code for inline/template definitions.
# .pcm contains module interface for imports.
#

printf "Phase 1: Compiling module interfaces (.o + .pcm)\n\n"

# ---- merged module A ----
# A.cppm exports a module and contains all code.
# Generates A.o (object code) and A.pcm (interface).
printf "Compiling module A (merged)\n\n"
clang++ "${CXXFLAGS[@]}" -c "$MOD1/A.cppm" -o "$BUILD/A.o" -fmodule-output

# ---- merged module B ----
# B.cppm imports A, contains all code.
# Finds A.pcm via -fprebuilt-module-path.
printf "Compiling module B (merged)\n\n"
clang++ "${CXXFLAGS[@]}" -c "$MOD1/B.cppm" -o "$BUILD/B.o" -fmodule-output

# ---- split module C (interface only) ----
# -x c++-module marks .cxx as module interface unit.
printf "Compiling module C interface\n\n"
clang++ "${CXXFLAGS[@]}" -x c++-module -c "$MOD1/C.cxx" -o "$BUILD/C.o" -fmodule-output

# ---- split module D (interface only) ----
# -x c++-module marks .cxx as module interface unit.
printf "Compiling module D interface\n\n"
clang++ "${CXXFLAGS[@]}" -x c++-module -c "$MOD1/D.cxx" -o "$BUILD/D.o" -fmodule-output

# ==================================================
# PHASE 2 — MODULE IMPLEMENTATIONS → .o
# ==================================================
#
# Implementation files contain definitions only.
# They import their own module via compiled .pcm.
# No .pcm files are generated here.
#

printf "\nPhase 2: Compiling module implementations (.o only)\n\n"

# ---- split module C implementation ----
# C.cpp implements functions declared in C.cxx.
# Finds C.pcm via -fprebuilt-module-path.

printf "Compiling module C implementation\n\n"
clang++ "${CXXFLAGS[@]}" -c "$MOD1/C.cpp" -o "$BUILD/C_impl.o"

# ---- split module D implementation ----

printf "Compiling module D implementation\n\n"
clang++ "${CXXFLAGS[@]}" -c "$MOD1/D.cpp" -o "$BUILD/D_impl.o"

# ==================================================
# PHASE 3 — CONSUMER FILES → .o
# ==================================================
#
# Regular translation units that import modules.
# Can also include traditional headers.
#

printf "\nPhase 3: Compiling consumer files\n\n"

# ---- C++ consumer (imports modules) ----

printf "Compiling Main.cpp (C++ consumer)\n\n"
clang++ "${CXXFLAGS[@]}" -c Main.cpp -o "$BUILD/Main_cpp.o"

# ---- C entry point (no modules) ----

printf "Compiling Main.c (C entry point)\n\n"
clang "${CFLAGS[@]}" -c Main.c -o "$BUILD/Main.o" -I"./"
