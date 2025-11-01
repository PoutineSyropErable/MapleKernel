from typing import List, Dict, Optional, Tuple
import os, sys
import numpy as np
import subprocess, time


from capstone import *

BIT = 32
if BIT == 16:
    ORIGINAL_FUNCTION = "add1616_start"
    CODE_START = 0xB040
elif BIT == 32:
    ORIGINAL_FUNCTION = "resume32"
    CODE_START = 0xB0A8

else:
    print("16 or 32 bit")
    sys.exit(1)

FILE = f"./code{BIT}_hex.txt"


def dissassemble_hex_str():

    with open(FILE, "r") as f:
        text = f.read()

    lines = text.split("\n")
    bin_array: List[int] = []

    for i, line in enumerate(lines):
        line = line.strip()
        try:
            value = int(line, 16)  # convert from hex string to integer
            bin_array.append(value)
        except Exception as e:
            print(f"exited early {i} out of {len(lines)}")

    bin_data = bytearray()
    for i, value in enumerate(bin_array):
        # convert each 32-bit integer to 4 bytes little-endian
        bin_data += value.to_bytes(4, byteorder="little")

    # write to file
    with open(f"code{BIT}_hex.bin", "wb") as f:
        f.write(bin_data)

    print(f"Binary written: {len(bin_data)} bytes")

    cmd = ["ndisasm", "-b", "16", f"code{BIT}_hex.bin"]
    # run and capture stdout
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout)

    return


def quick():
    # Read the binary you produced
    with open(f"code{BIT}_hex.bin", "rb") as f:
        code = f.read()

    # Initialize Capstone for 16-bit x86
    if BIT == 16:
        mode = CS_MODE_16
    if BIT == 32:
        mode = CS_MODE_32
    else:
        sys.exit(2)

    md = Cs(CS_ARCH_X86, mode)
    md.detail = True  # Optional: allows more info about operands

    # Disassemble
    for instr in md.disasm(code, CODE_START):  # 0xB800 is just an example address
        print(f"0x{instr.address:X}:\t{instr.mnemonic}\t{instr.op_str}")


# dissassemble_hex_str()
quick()
