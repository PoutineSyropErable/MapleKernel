#!/bin/bash
set -e

printf -- "\n\n\n===========Cleaning============\n\n\n"
./clean.sh
printf -- "\n\n\n===========Headers============\n\n\n"
./headers.sh
printf -- "\n\n\n===========Iso============\n\n\n"
./iso.sh

printf -- "\n\n\n===========qemu============\n\n\n"
./qemu.sh &
# qemu-system-x86_64 -drive format=raw,file=hello.bin &
QEMU_PID=$!

# Give QEMU a second to start up
sleep 1

printf -- "\n\n\n===========VNC seeing============\n\n\n"
# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null
