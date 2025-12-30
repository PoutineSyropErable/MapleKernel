#!/usr/bin/env bash
set -euo pipefail

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "Building in: $SCRIPT_DIR"

cd "$SCRIPT_DIR"

function find_git_root() {
	local dir=${1:-$PWD} # start from given dir or current directory
	while [[ "$dir" != "/" ]]; do
		if [[ -d "$dir/.git" ]]; then
			echo "$dir"
			return 0
		fi
		dir=$(dirname "$dir")
	done
	echo "No git repository found" >&2
	return 1
}

project_root=$(find_git_root)

BUILD_DIR="$project_root/build32"
zig build "$@"

# mv ./zig-out/lib/libkernel_zig.a ../../build/libkernel_zig.a
mv ./zig-out/kernel_zig.o "$BUILD_DIR/kernel_zig.o"
