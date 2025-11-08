# In GDB, define this Python function
python


def print_args16_offsets():
    fields = [
        "gdt_root",
        "esp",
        "ret1",
        "ret2",
        "func",
        "func_cs",
        "argc",
        "func_args[0]",
        "func_args[1]",
        "func_args[2]",
        "func_args[3]",
        "func_args[4]",
        "func_args[5]",
        "func_args[6]",
        "func_args[7]",
        "func_args[8]",
        "func_args[9]",
        "func_args[10]",
        "func_args[11]",
    ]
    for f in fields:
        offset = gdb.parse_and_eval(f"(char*)&args16_start.{f} - (char*)&args16_start")
        print(f"{f:12} -> {int(offset)} bytes")


end
