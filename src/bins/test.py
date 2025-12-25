#!/usr/bin/env python3
import sys
import re

import struct

from typing import List, Tuple


def parse_nm_output(filename) -> List[Tuple[int, str]]:
    """Parse nm output file and extract addresses and function names."""
    symbols = []

    try:
        with open(filename, "r") as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if not line:
                    continue

                # Split by whitespace
                parts = line.split()

                if len(parts) < 2:
                    print(f"Warning: Line {line_num}: Unexpected format: {line}")
                    exit(1)
                    continue

                # First part is the address (hex)
                address_str = parts[0]
                # Last part is the symbol type (T, B, R, D, etc.)
                symbol_type = parts[1]
                # The rest is the function name (might contain spaces for C++ names)
                # Join everything from part 2 onward
                if len(parts) < 3:
                    print(f"Warning: Line {line_num}: No function name: {line}")
                    exit(2)
                    continue

                name = " ".join(parts[2:])

                # Try to parse address
                try:
                    address = int(address_str, 16)
                    symbols.append((address, name))
                except ValueError:
                    print(f"Warning: Line {line_num}: Invalid address '{address_str}'")
                    exit(3)
                    continue

    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        return []

    return symbols


def write_symbols_binary(symbols: List[Tuple[int, str]], output_file: str = "symbols.bin"):
    """Write symbols to binary file with format:
    consecutive addresses (4 bytes each, little-endian)
    then "8\n" as separator
    then consecutive null-terminated names
    """
    if not symbols:
        print("No symbols to write.")
        return False

    # Sort by address
    symbols.sort(key=lambda x: x[0])

    try:
        with open(output_file, "wb") as f:
            # 1. Write all addresses (4 bytes each, little-endian)
            for address, _ in symbols:
                f.write(struct.pack("<I", address))  # 32-bit little-endian

            # 2. Write separator: ASCII '8' followed by newline
            f.write(b"8\n")

            # 3. Write all names as null-terminated strings
            for _, name in symbols:
                # Encode name as UTF-8 and add null terminator
                name_bytes = name.encode("utf-8")
                f.write(name_bytes)
                f.write(b"\x00")

        # Calculate file statistics
        addr_section_size = len(symbols) * 4  # 4 bytes per address
        separator_size = 2  # '8' (1 byte) + '\n' (1 byte)

        # Calculate names section size
        names_size = 0
        for _, name in symbols:
            names_size += len(name.encode("utf-8")) + 1  # +1 for null terminator

        total_size = addr_section_size + separator_size + names_size

        print(f"Successfully wrote {len(symbols)} symbols to {output_file}")
        print(f"  Address section: {addr_section_size} bytes")
        print(f"  Separator: {separator_size} bytes")
        print(f"  Names section: {names_size} bytes")
        print(f"  Total file size: {total_size} bytes")

        return True

    except Exception as e:
        print(f"Error writing to {output_file}: {e}")
        return False


def main():
    if len(sys.argv) != 2:
        print("Usage: python parse_nm.py <nm_output.txt>")
        print("Example: python parse_nm.py nm_output.txt")
        sys.exit(1)

    filename = sys.argv[1]
    symbols = parse_nm_output(filename)

    if not symbols:
        print("No symbols found or error reading file.")
        sys.exit(1)

    print(f"Found {len(symbols)} symbols:")
    print("-" * 100)

    # Print all symbols
    for address, name in symbols:
        print(f"0x{address:08x}, {name}")

    print("-" * 100)
    print(f"Total: {len(symbols)} symbols")

    write_symbols_binary(symbols, "symbols.bin")


if __name__ == "__main__":
    main()
