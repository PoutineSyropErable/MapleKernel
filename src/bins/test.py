#!/usr/bin/env python3
import sys
import re

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


if __name__ == "__main__":
    main()
