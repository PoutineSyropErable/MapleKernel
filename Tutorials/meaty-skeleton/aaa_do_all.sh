#!/bin/bash
set -e

printf -- "\n\n\n===========Cleaning============\n\n\n"
./clean.sh
printf -- "\n\n\n===========Headers============\n\n\n"
./headers.sh

printf -- "\n\n\n===========Iso============\n\n\n"
./iso.sh

# skip QEMU, and iso when building for bear
if [[ "$1" != "--bear" ]]; then

	printf -- "\n\n\n===========qemu============\n\n\n"
	./qemu.sh
	# qemu-system-x86_64 -drive format=raw,file=hello.bin &
fi

# vnc is in qemu.sh
