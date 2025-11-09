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


stack_start = 0x7000
stack_start = 0x10000
stack_size = 16 * 1024
stack_end = stack_start + stack_size

args16_start = stack_end + 16
args16_end = args16_start + 16

call_real16_function = args16_end + 16


simple2(stack_start, stack_end, "stack16")
simple2(args16_start, args16_end, "args16")

simple2(call_real16_function, call_real16_function + 2048, "call_real16_function")
# 2048 is a random size

print("\n")
simple(1024, "1024")
simple(1024 * 16, "1024*16")


# :MEMORY_LOCATION, memory location, memoryLocation.
# stack16         | 0x7000     - 0xafff     | 28.00 KB - 44.00 KB | 28672 - 45055
# args16          | 0xb000     - 0xb00f     | 44.00 KB - 44.01 KB | 45056 - 45071
# call_real16_function   |      0xb010             |        44.02 KB     | 45072-?
# That's the correct address. Nice!
