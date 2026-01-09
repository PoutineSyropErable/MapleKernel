#!/usr/bin/env bash

set -eou pipefail

# If you have qemu full
QEMU_FULL=false
# CONTROL + ALT + G to get the mouse back (GTK), default on my machine
# or CTRL+ALT (SDL)

MOV_WORKSPACE=12 # In hyprland, move it there
MOV_PID_DIR="$HOME/.local/bin"

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
src="$project_root/src/"
cd "$src" || { # Tries to cd to "src" relative to current dir
	echo "Could not cd to $src."
	exit 1
}
mkdir -p "$MOV_PID_DIR" && { [[ -e "$MOV_PID_DIR/move_pid_to_workspace" ]] || ln -s "$project_root/src/user_tools/move_pid_to_workspace.py" "$MOV_PID_DIR/move_pid_to_workspace"; }
# this script might not work on something else, heh, who cares. Make your own or move it yourself

DEBUG_OR_RELEASE="${1:-release}"
QEMU_OR_REAL_MACHINE="${2:-qemu}"
MACHINE_BITNESS="${3:-64}"
MOVE_VNC="${4:-move}"

BUILD32_DIR="../build32"
BUILD64_DIR="../build64"
ISO_DIR="../isodir"

./LongMode/build64.sh "$DEBUG_OR_RELEASE" "$QEMU_OR_REAL_MACHINE"
./write_kernel64_size_to_header.sh
./ProtectedMode/build32.sh "$DEBUG_OR_RELEASE" "$QEMU_OR_REAL_MACHINE"
# Check if the kernel is multiboot-compliant
USE_MULTIBOOT1=false
if [ "$USE_MULTIBOOT1" == true ]; then
	if grub-file --is-x86-multiboot "$BUILD32_DIR/kernel32.elf"; then
		echo "Multiboot confirmed"
	else
		echo "The file is not multiboot"
		exit 1
	fi
else
	if grub-file --is-x86-multiboot2 "$BUILD32_DIR/kernel32.elf"; then
		echo "Multiboot2 confirmed"
	else
		echo "The file is not multiboot 2"
		exit 1
	fi

fi

# Copy the kernel binary and GRUB configuration to the ISO directory

mkdir -p "$ISO_DIR/boot/grub"
mkdir -p "$ISO_DIR/EFI/BOOT"

cp "grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"
# Copy the kernel into the ISO tree

# Build standalone GRUB
grub-mkstandalone \
	-O x86_64-efi \
	--modules="part_gpt part_msdos multiboot2 normal configfile" \
	--fonts="" \
	--locales="" \
	--themes="" \
	-o "$ISO_DIR/EFI/BOOT/BOOTX64.EFI" \
	"boot/grub/grub.cfg=$ISO_DIR/grub.cfg"

# Create the ISO image
RELEASE_DIR="../release"
mkdir -p "$RELEASE_DIR"
grub-mkrescue -o "$RELEASE_DIR/myos.iso" "$ISO_DIR"
echo "ISO created successfully: $RELEASE_DIR/myos.iso"

# Check if Ventoy USB is mounted
VENTOY_PATH="/run/media/$USER/Ventoy"
if [ -d "$VENTOY_PATH" ]; then
	echo "Ventoy detected at $VENTOY_PATH. Copying ISO..."
	cp "$RELEASE_DIR/myos.iso" "$VENTOY_PATH/"
	echo "ISO copied to Ventoy USB successfully."
else
	echo "Ventoy USB not found at $VENTOY_PATH. Skipping copy."
fi

# Create 64 MB raw disk image

# start qemu using the binary, bypassing grub.
# -no-reboot: don't reset if the kernel crash
# -d in_asm, int, cpu_reset: Logs every instruction executed by cpu, every interupt and cpu reset events
# -D qemu_instr.log : The log file (redirect -d)
# -serial stdio: redirect COM1 serial port to your terminal

# ===== Pick one of those two
# -kernel "$BUILD_DIR/kernel32.elf" \
# -cdrom "$BUILD_DIR/myos.iso" \

if [[ "$QEMU_OR_REAL_MACHINE" != "qemu" ]]; then
	printf -- "\nSince Not In Qemu, then we aren't gonna emulate the machine\n\n"
	exit 1
fi

USE_IMAGE=false
if [ "$USE_IMAGE" == true ]; then
	bochs -f bochsrc.txt
else
	QEMU32=qemu-system-i386
	QEMU64=qemu-system-x86_64

	# Choose QEMU based on machine bitness
	if [[ "$MACHINE_BITNESS" == "64" ]]; then
		QEMU="$QEMU64"
		printf "\n\nOn a 64 bit machine\n\n"
	else
		QEMU="$QEMU32"
		printf "\n\nOn a 32 bit machine\n\n"
	fi

	# Check if QEMU is already running, kill only if it exists
	if pgrep -f "$QEMU" >/dev/null 2>&1; then
		echo "Killing existing QEMU process..."
		pkill -f "$QEMU" 2>/dev/null
		sleep 1 # Give it time to die
	fi

	QEMU_CPU_FLAG=()
	CPU_MODEL=""
	if [[ "$MACHINE_BITNESS" == "64" ]]; then
		CPU_MODEL="host"
		CPU_MODEL="qemu64"
	else
		CPU_MODEL="host"
		CPU_MODEL="qemu32"
	fi

	if [[ "$CPU_MODEL" == "host" ]]; then
		QEMU_CPU_FLAG+=("--enable-kvm")
	fi

	# False for now, as I'm making an MMIO apic based driver
	ENABLE_X2APIC=false

	# conditional addition
	if [ "$ENABLE_X2APIC" = true ]; then
		CPU_MODEL="${CPU_MODEL},+x2apic" # note: no spaces
	fi

	CPU_MODEL="${CPU_MODEL},+avx" # note: no spaces

	QEMU_CPU_FLAG+=("-cpu" "$CPU_MODEL")

	# Array of QEMU debug options
	ENABLE_QEMU_DEBUG=true
	DEBUG_LOG_OPTS=()

	# Check if debug flag is set
	if [[ "$ENABLE_QEMU_DEBUG" == true ]]; then
		DEBUG_LOG_OPTS+=(
			"-d" "in_asm,int,cpu_reset"
			"-D" "qemu_instr.log"
		)
	fi

	printf -- "\n\n===========Executing with %s ===========\n\n" "$QEMU"

	USE_VNC_FLAG=()
	if [[ "$QEMU_FULL" == "false" ]]; then
		USE_VNC_FLAG+=("-vnc" ":0")
	fi

	# TODO: Make -d and -D conditional.
	$QEMU \
		-cdrom "$RELEASE_DIR/myos.iso" \
		-no-reboot \
		"${QEMU_DBG_FLAGS[@]}" \
		"${DEBUG_LOG_OPTS[@]}" \
		-smp 12 \
		"${QEMU_CPU_FLAG[@]}" \
		"${USE_VNC_FLAG[@]}" \
		-serial stdio &
	# -vga vmware \

	QEMU_PID=$!
	if [[ "$QEMU_FULL" == "true" ]]; then
		if [[ "$MOVE_VNC" == "move" ]]; then
			# this is a ~/.local/bin script
			move_pid_to_workspace $QEMU_PID "$MOV_WORKSPACE"
		fi

	else
		# Give QEMU a second to start up
		sleep 1

		vncviewer localhost:5900 &
		VNC_PID=$!

		# sleep 1
		if [[ "$MOVE_VNC" == "move" ]]; then
			move_pid_to_workspace $VNC_PID "$MOV_WORKSPACE"
		fi

		wait $VNC_PID

		# After you close the VNC viewer, kill QEMU
		kill $QEMU_PID 2>/dev/null

	fi

	# sleep 1

fi
