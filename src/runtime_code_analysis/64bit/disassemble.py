#!/usr/bin/env python3
import struct
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone.x86 import X86_OP_IMM, X86_OP_REG, X86_OP_MEM


def load_binary_from_text(filename):
    """
    Load binary data from text file where each line contains a uint32_t in string format
    """
    binary_data = bytearray()

    with open(filename, "r") as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue

            try:
                # Parse the uint32_t value
                value = int(line.strip())

                # Validate it's a valid uint32_t
                if value < 0 or value > 0xFFFFFFFF:
                    print(f"Warning: Line {line_num}: Value {hex(value)} out of uint32_t range")

                # Convert to little-endian bytes (assuming x86)
                binary_data.extend(struct.pack("<I", value & 0xFFFFFFFF))

            except ValueError as e:
                print(f"Error on line {line_num}: '{line}' - {e}")
                continue

    return bytes(binary_data)


def disassemble_64bit_code(binary_data, base_address=0x100000):
    """
    Disassemble 64-bit code using Capstone
    """
    # Initialize Capstone for x86-64
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True  # Enable detailed information for operands

    print(f"Binary size: {len(binary_data)} bytes ({len(binary_data)//4} dwords)")
    print(f"Starting disassembly at address: 0x{base_address:016x}")
    print("=" * 80)

    # Track statistics
    instruction_count = 0
    total_bytes = 0

    for instruction in md.disasm(binary_data, base_address):
        instruction_count += 1
        total_bytes += instruction.size

        # Format address
        address_str = f"0x{instruction.address:016x}"

        # Format bytes
        bytes_str = " ".join(f"{b:02x}" for b in instruction.bytes)
        bytes_str = bytes_str.ljust(24)

        # Get instruction details
        mnemonic = instruction.mnemonic
        op_str = instruction.op_str

        # Try to get detailed operands
        try:
            if md.detail and instruction.operands:
                # You can add more detailed operand analysis here
                pass
        except:
            pass

        print(f"{address_str}:  {bytes_str}  {mnemonic:8} {op_str}")

    print("=" * 80)
    print(f"Total instructions: {instruction_count}")
    print(f"Total bytes processed: {total_bytes}")
    print(f"Remaining bytes: {len(binary_data) - total_bytes}")

    return md


def find_code_entry_points(binary_data, md, base_address=0x100000):
    """
    Try to find potential entry points (like jumps, calls)
    """
    print("\n" + "=" * 80)
    print("Looking for potential entry points (calls, jumps, etc.):")
    print("=" * 80)

    interesting_mnemonics = {"call", "jmp", "jne", "je", "jg", "jl", "jge", "jle", "ja", "jb"}

    for instruction in md.disasm(binary_data, base_address):
        if instruction.mnemonic in interesting_mnemonics:
            address_str = f"0x{instruction.address:016x}"
            bytes_str = " ".join(f"{b:02x}" for b in instruction.bytes)

            # Check if it's a direct jump/call
            if instruction.op_str.startswith("0x"):
                target_addr = int(instruction.op_str, 16)
                print(f"{address_str}:  {bytes_str:24}  {instruction.mnemonic:8} {instruction.op_str}")
                print(f"    -> Direct target: 0x{target_addr:016x}")


def dump_memory_regions(binary_data, chunk_size=32, base_address=0x100000):
    """
    Dump memory in hex format for inspection
    """
    print("\n" + "=" * 80)
    print("Memory dump (first 256 bytes):")
    print("=" * 80)

    for i in range(0, min(256, len(binary_data)), 16):
        addr = base_address + i
        hex_part = " ".join(f"{b:02x}" for b in binary_data[i : i + 16])
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in binary_data[i : i + 16])
        print(f"0x{addr:016x}:  {hex_part:47}  |{ascii_part}|")


def main():
    import sys

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input_file.txt> [base_address]")
        print(f"Example: {sys.argv[0]} kernel_dump.txt 0xffffffff80000000")
        sys.exit(1)

    filename = sys.argv[1]
    base_address = 0x100000  # Default base

    if len(sys.argv) > 2:
        try:
            base_address = int(sys.argv[2], 0)
        except ValueError:
            print(f"Invalid base address: {sys.argv[2]}")
            sys.exit(1)

    print(f"Loading binary from: {filename}")
    print(f"Using base address: 0x{base_address:016x}")

    try:
        # Load the binary data
        binary_data = load_binary_from_text(filename)

        if not binary_data:
            print("No data loaded!")
            sys.exit(1)

        print(f"Successfully loaded {len(binary_data)} bytes")

        # Optional: Dump memory regions
        dump_memory_regions(binary_data, base_address=base_address)

        # Disassemble
        print("\n" + "=" * 80)
        print("DISASSEMBLY:")
        print("=" * 80)

        md = disassemble_64bit_code(binary_data, base_address)

        # Look for interesting patterns
        find_code_entry_points(binary_data, md, base_address)

        # Save to file
        output_file = filename.replace(".txt", "_disasm.txt")
        with open(output_file, "w") as f:
            f.write(f"Disassembly of {filename}\n")
            f.write(f"Base address: 0x{base_address:016x}\n")
            f.write(f"Total size: {len(binary_data)} bytes\n")
            f.write("=" * 80 + "\n")

            # Redirect disassembly to file
            import io
            import sys

            old_stdout = sys.stdout
            sys.stdout = io.StringIO()

            disassemble_64bit_code(binary_data, base_address)

            output = sys.stdout.getvalue()
            sys.stdout = old_stdout

            f.write(output)

        print(f"\nDisassembly saved to: {output_file}")

    except Exception as e:
        print(f"Error: {e}")
        import traceback

        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
