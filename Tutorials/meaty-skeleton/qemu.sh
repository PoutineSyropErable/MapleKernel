#!/bin/sh
set -e
. ./iso.sh

qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso &

QEMU_PID=$!
# Capture QEMU's PID

# Give QEMU a second to start up
sleep 1

printf -- "\n\n\n===========VNC seeing============\n\n\n"
# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null
