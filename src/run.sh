#!/bin/bash

set -eou pipefail
ARG1="${1:-}"

# Define directories
BUILD_DIR="build"

DEBUG_OPT_LVL="-O0"
RELEASE_OPT_LVL="-O0"
QEMU_DBG_FLAGS=()

if [[ "$ARG1" == "debug" ]]; then
	QEMU_DBG_FLAGS+=("-s" "-S")
else
	echo "In normal mode, $RELEASE_OPT_LVL optimisation"
fi

qemu-system-i386 \
	-cdrom "$BUILD_DIR/myos.iso" \
	-no-reboot \
	"${QEMU_DBG_FLAGS[@]}" \
	-d in_asm,int,cpu_reset \
	\
	-serial stdio & # -D qemu_instr.log \

QEMU_PID=$!

# Give QEMU a second to start up
sleep 1

# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null
