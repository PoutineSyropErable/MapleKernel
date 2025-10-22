print("\n\n====Start of program======\n\n")


def format_kb(address):
    kb = address / 1024
    return f"{kb:.2f} KB"


def simple(address: int, name: str):
    print(f"{name} = {hex(address)}, {format_kb(address)}, {address}")


def simple2(address_start: int, address_end: int, name: str):
    s1 = f"{name:<35}"  # left-align, width 35
    s2 = f"{hex(address_start):<10} - {hex(address_end):<10}"
    s3 = f"{format_kb(address_start):<8} - {format_kb(address_end):<8}"
    s4 = f"{address_start:<10} - {address_end:<10}"
    print(f"{s1} | {s2} | {s3} | {s4}")


start = 0x7C00
size = 446
end_of_mini_bootloader = start + size

end_of_mbr = end_of_mini_bootloader + 66
# simple(start, "start")
# simple(start + size, "end of mini boot loader")
# simple(start + size + 66, "end of mbr")


print("\n")
simple2(start, start + size + 66, "mini boot")


print("")
simple2(0x08000, 0x0FFFF, "boot loader 16")
simple2(0x10000, 0x11FFF, "boot loader 16 stack")


print("")
simple2(0x20000, 0x2FFFF, "boot chain 32")
simple2(0x30000, 0x3FFFF, "boot chain 32 stack")


print("")
simple2(0x40000, 0x5FFFF, "32 bit transformer")
simple2(0x60000, 0x7FFFF, "32 bit transformer stack")


print("")
simple2(0x100000, 0x200000, "64 bit kernel")
