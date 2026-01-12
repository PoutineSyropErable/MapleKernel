const std = @import("std");
const file2 = @import("file2.zig");

// CORRECT panic signature for Zig 0.11+
pub fn panic(msg: []const u8, error_return_trace: ?*@import("std").builtin.StackTrace, ret_addr: ?usize) noreturn {
    _ = msg;
    _ = error_return_trace;
    _ = ret_addr;

    // Halt CPU
    while (true) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}

export fn kernel64_zig_main() noreturn {
    // Your kernel code here

    // Setup your kernel...

    const value: i32 = file2.zig_add(5, 6);
    _ = value + 1;

    // Never return
    while (true) {
        // asm volatile ("mov rax, 15");
        asm volatile ("hlt");
    }
}
