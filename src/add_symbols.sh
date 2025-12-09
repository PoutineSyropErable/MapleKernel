#!/usr/bin/env bash
BUILD_DIR="./build"

printf "\n\n====== Embedding Symbol Tables =====\n\n"

# Step 1: Find where .bss ends to place symbols after it
BSS_END=$(readelf -S "$BUILD_DIR/myos.elf" | grep "\.bss" | awk '{print "0x"$5}')
if [ -z "$BSS_END" ]; then
	# If .bss not found, use default
	BSS_END=0x300000
else
	# Add 4KB padding after .bss
	BSS_END=$(printf "0x%x" $((BSS_END + 0x1000)))
fi

echo "Placing symbols at: $BSS_END"

# Step 2: Use objcopy to change section flags and addresses
objcopy \
	--set-section-flags .symtab=alloc,contents,load,readonly \
	--set-section-flags .strtab=alloc,contents,load,readonly \
	--change-section-address .symtab=$BSS_END \
	--change-section-address .strtab=$(printf "0x%x" $((BSS_END + 0x10000))) \
	"$BUILD_DIR/myos.elf" "$BUILD_DIR/myos_with_syms.elf"

# Step 3: Convert to final binary
objcopy -O binary "$BUILD_DIR/myos_with_syms.elf" "$BUILD_DIR/myos.bin"

# Step 4: Verify symbols are loaded
echo "Verifying symbol table placement:"
readelf -S "$BUILD_DIR/myos_with_syms.elf" | grep -A1 -B1 "symtab\|strtab"
