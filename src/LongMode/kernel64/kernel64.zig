// const std = @import("std");

const file2 = @import("file2.zig");
const builtin = @import("builtin");
const std = @import("std");

const std_options = @import("std_options.zig");

// Include std_options to customize std

// Optional: Also export a direct panic function
pub const panic = std.debug.FullPanic(std_options.kernel_panic_handler);

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
