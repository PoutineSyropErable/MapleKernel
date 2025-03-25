#!/bin/bash 

./clean.sh
./headers.sh
./iso.sh

./qemu.sh & 
# qemu-system-x86_64 -drive format=raw,file=hello.bin &
QEMU_PID=$!

# Give QEMU a second to start up
sleep 1

# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null
