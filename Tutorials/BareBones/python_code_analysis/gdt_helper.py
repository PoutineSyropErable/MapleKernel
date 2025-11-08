#!/usr/env/bin python
from typing import List, Dict, Tuple, Optional
from enum import Enum


class Color(Enum):
    red = "red"
    blue = "blue"
    green = "green"
    yellow = "yellow"
    cyan = "cyan"
    white = "white"
    magenta = "magenta"


COLORS_STRING = {
    "red": "\033[31m",
    "green": "\033[32m",
    "yellow": "\033[33m",
    "blue": "\033[34m",
    "magenta": "\033[35m",
    "cyan": "\033[36m",
    "white": "\033[37m",
}

RESET = "\033[0m"


def color_string(string: str, color: Color):
    return f"{COLORS_STRING[color.value]}{string}{RESET}"


# The value of cs: 16
#
# The value of ss: 24
#
# The value of ds: 24
#
# The value of es: 24
#
# The value of fs: 24
#
# The value of gs: 24
# gdt base address = 10B0
# gdt size limit = 32
#
# The gdtr values:
# gdt[0].low = 0
# gdt[0].high = 0
# gdt[1].low = 0
# gdt[1].high = 0
# gdt[2].low = FFFF
# gdt[2].high = CF9A00
# gdt[3].low = FFFF
# gdt[3].high = CF9300

from ctypes import c_uint32, c_uint64


class GDT_ENTRY:
    def __init__(self, low: int, high: int):
        self.low = c_uint32(low)
        self.high = c_uint32(high)

    # ---- Getters ----
    @property
    def limit_low(self):
        return self.low.value & 0xFFFF

    @property
    def base_low(self):
        return (self.low.value >> 16) & 0xFFFF

    @property
    def base_mid(self):
        return self.high.value & 0xFF

    @property
    def access(self):
        """P, DPL, S, Type"""
        return (self.high.value >> 8) & 0xFF

    @property
    def limit_high(self):
        return (self.high.value >> 16) & 0xF

    @property
    def flags(self):
        """G, D/B, L, AVL"""
        return (self.high.value >> 20) & 0xF

    @property
    def base_high(self):
        return (self.high.value >> 24) & 0xFF

    @property
    def limit(self):
        """Full 20-bit segment limit"""
        return (self.limit_high << 16) | self.limit_low

    @property
    def base(self):
        """Full 32-bit base address"""
        return (self.base_high << 24) | (self.base_mid << 16) | self.base_low

    # -------- Access bits --------
    @property
    def accessed(self):
        return bool(self.access & 0x01)

    @property
    def rw(self):
        return bool((self.access >> 1) & 0x01)

    @property
    def dc(self):
        return bool((self.access >> 2) & 0x01)

    @property
    def executable(self):
        return bool((self.access >> 3) & 0x01)

    @property
    def descriptor_type(self):
        return bool((self.access >> 4) & 0x01)  # 0=system, 1=code/data

    @property
    def dpl(self):
        return (self.access >> 5) & 0x03

    @property
    def present(self):
        return bool((self.access >> 7) & 0x01)

        # -------- Flags bits --------

    @property
    def avl(self):
        return bool(self.flags & 0x1)

    @property
    def long_mode(self):
        return bool((self.flags >> 1) & 0x1)

    @property
    def default_big(self):
        return bool((self.flags >> 2) & 0x1)

    @property
    def granularity(self):
        return bool((self.flags >> 3) & 0x1)

    # -------- Derived meaning --------
    @property
    def segment_type(self):
        if not self.descriptor_type:
            return "System"
        if self.executable:
            return "Code"
        else:
            return "Data"

    @property
    def default_size(self):
        if self.long_mode:
            return "64-bit"
        return "32-bit" if self.default_big else "16-bit"

    # ---- Setters ----
    @limit.setter
    def limit(self, value):
        value &= 0xFFFFF
        low = self.low.value
        high = self.high.value
        low = (low & 0xFFFF0000) | (value & 0xFFFF)
        high = (high & 0xFFF0FFFF) | ((value >> 16) << 16)
        self.low = c_uint32(low)
        self.high = c_uint32(high)

    @base.setter
    def base(self, value):
        value &= 0xFFFFFFFF
        low = self.low.value
        high = self.high.value
        low = (low & 0x0000FFFF) | ((value & 0xFFFF) << 16)
        high = (high & 0x0000FFFF) | (((value >> 16) & 0xFF) << 0) | (((value >> 24) & 0xFF) << 24)
        self.low = c_uint32(low)
        self.high = c_uint32(high)

    def set_low_bit(self, bit_number: int, value: int):
        if value not in (0, 1):
            raise ValueError("value must be 0 or 1")

        low = self.low.value

        if value == 0:
            # Clear the bit
            low = low & ~(1 << bit_number)
        elif value == 1:
            # Set the bit
            low = low | (1 << bit_number)

        self.low = c_uint32(low)

    def set_high_bit(self, bit_number: int, value: int):
        if value not in (0, 1):
            raise ValueError("value must be 0 or 1")

        high = self.high.value

        if value == 0:
            # Clear the bit
            high = high & ~(1 << bit_number)
        elif value == 1:
            # Set the bit
            high = high | (1 << bit_number)

        self.high = c_uint32(high)

    def set_default_operand_size(self, op_size_bool: int):
        if op_size_bool not in (0, 1):
            raise ValueError("value must be 0 or 1")
        self.set_high_bit(22, op_size_bool)

    def set_granularity(self, granularity: int):
        if granularity not in (0, 1):
            raise ValueError("value must be 0 or 1")
        self.set_high_bit(23, granularity)

    # --- improved readable representation ---

    def __repr__(self):
        desc = []

        desc.append(f"{self.segment_type} Segment")
        if self.present:
            desc.append("Present")
        else:
            desc.append("Not Present")

        desc.append(f"DPL={self.dpl}")
        desc.append(f"Base=0x{self.base:08X}")
        desc.append(f"Limit=0x{self.limit:05X}")

        # segment-specific flags
        if self.segment_type == "Code":
            desc.append("Executable")
            if self.rw:
                desc.append("Readable")
            if self.dc:
                desc.append("Conforming")
        elif self.segment_type == "Data":
            if self.rw:
                desc.append("Writable")
            if self.dc:
                desc.append("Expand-Down")

        # flags and mode
        desc.append(self.default_size)
        desc.append("G=4KiB" if self.granularity else "G=Byte")

        return "<" + ", ".join(desc) + ">"

    def hex_qword_simple(self):
        """Simply concatenate high and low as hex, zero-padded."""
        # Convert to ints if they are bytes
        low_int = self.low.value
        high_int = self.high.value

        flags = "G, D/B, L, AVL"
        access = "P, DPL, S, TYPE"
        return f"0x{high_int:08X}_{low_int:08X}"

    def hex_qword_colored(self):
        """Return the 64-bit GDT entry as a hex string with each section colored."""

        # Extract the main sections
        sections: Dict[str, int] = {
            "limit_low": self.limit_low,
            "base_low": self.base_low,
            "base_mid": self.base_mid,
            "access": self.access,
            "limit_high": self.limit_high,
            "flags": self.flags,
            "base_high": self.base_high,
        }

        # Assign colors for each section
        section_colors: Dict[str, Color] = {
            "limit_low": Color.red,
            "limit_high": Color.magenta,
            "base_low": Color.white,
            "base_mid": Color.yellow,
            "base_high": Color.green,
            "access": Color.blue,
            "flags": Color.cyan,
        }

        # Build the hex string with colors
        hex_parts: List[str] = []

        # low 32 bits
        hex_parts.append(color_string(f"{sections['limit_low']:04X}", section_colors["limit_low"]))
        hex_parts.append(color_string(f"{sections['base_low']:04X}", section_colors["base_low"]))
        hex_parts.append("_")  # separator for readability

        # high 32 bits
        hex_parts.append(color_string(f"{sections['base_mid']:02X}", section_colors["base_mid"]))
        hex_parts.append(color_string(f"{sections['access']:02X}", section_colors["access"]))
        hex_parts.append(color_string(f"{sections['limit_high']:01X}", section_colors["limit_high"]))
        hex_parts.append(color_string(f"{sections['flags']:01X}", section_colors["flags"]))
        hex_parts.append(color_string(f"{sections['base_high']:02X}", section_colors["base_high"]))

        return "0x" + "".join(hex_parts)


print(f"\nStart of program\n")


code = GDT_ENTRY(0xFFFF, 0xCF9A00)
data = GDT_ENTRY(0xFFFF, 0xCF9300)

PRINT_CODE = False
PRINT_DATA = not PRINT_CODE

print("=====")
print(code.hex_qword_colored())
print(code)
print()

code.set_default_operand_size(0)
print(code)
print(code.hex_qword_colored())
print()

code.set_granularity(0)
print(code)
print(code.hex_qword_colored())
print()

code.limit = 0xFFFF
print(code)
print(code.hex_qword_colored())
print()

print("=====")
print(data.hex_qword_colored())
print(data)
print()

data.set_default_operand_size(0)
print(data)
print(data.hex_qword_colored())
print()

data.set_granularity(0)
print(data)
print(data.hex_qword_colored())
print()

data.limit = 0xFFFF
print(data)
print(data.hex_qword_colored())
print()


# c = 12 = 8 + 4
# c = 0000 1100


# type = data/code, conforming/expand down, Write permission, Accessed
