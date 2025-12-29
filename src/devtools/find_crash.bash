#!/bin/bash
# find_crash.sh
ADDR="0x0020c23d"
echo "Looking for address $ADDR..."

# Convert to decimal for nm
DEC_ADDR=$((ADDR))

# Use nm to find function boundaries
i686-elf-nm -n build/myos.elf | awk -v addr="$DEC_ADDR" '
  /^[0-9a-f]+ [Tt] / {
    if (strtonum("0x"$1) <= addr && strtonum("0x"$1) > last_addr) {
      last_addr = strtonum("0x"$1);
      last_func = $3;
    }
  }
  END {
    if (last_func) {
      printf "Address 0x%x is in function: %s\n", addr, last_func;
    } else {
      printf "Function not found for address 0x%x\n", addr;
    }
  }
'

# Also check with objdump for exact line
echo -e "\nDisassembly around crash:"
i686-elf-objdump -d --start-address=$((ADDR - 0x20)) --stop-address=$((ADDR + 0x20)) build/myos.elf
