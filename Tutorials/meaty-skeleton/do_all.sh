#!/bin/bash
set -e

printf -- "\n\n\n===========Cleaning============\n\n\n"
./clean.sh
printf -- "\n\n\n===========Headers============\n\n\n"
./headers.sh
printf -- "\n\n\n===========Iso============\n\n\n"
./iso.sh

printf -- "\n\n\n===========qemu============\n\n\n"
# qemu-system-x86_64 -drive format=raw,file=hello.bin &
./qemu.sh

# vnc is in qemu.sh
