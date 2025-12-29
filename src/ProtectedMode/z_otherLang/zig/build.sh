#!/usr/bin/env bash
set -euo pipefail

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "Building in: $SCRIPT_DIR"

cd "$SCRIPT_DIR"
zig build "$@"

# mv ./zig-out/lib/libkernel_zig.a ../../build/libkernel_zig.a
mv ./zig-out/kernel_zig.o ../../build/kernel_zig.o
