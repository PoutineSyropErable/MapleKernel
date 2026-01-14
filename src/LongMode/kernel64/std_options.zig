// std_options.zig - Customize std for kernel
const std = @import("std");
const stdio = @import("stdio");

/// Customize standard library behavior for kernel
/// This is something zig actually search for and use reflexion to modify its standard library behavior
pub const std_options: std.Options = .{
    // Disable segfault handler (kernel handles its own faults)
    .enable_segfault_handler = false,

    // Use kernel logging (to serial/console)
    .logFn = kernel_log,

    // Custom panic handler
    .panic_handler = kernel_panic_handler,
};

// Kernel logging to serial port
pub fn kernel_log(
    comptime level: std.log.Level,
    // comptime scope: @Type(.enum_literal),
    comptime format: []const u8,
    args: anytype,
) void {
    // Write to COM1 serial port

    // Prefix with log level
    const prefix = switch (level) {
        .err => "[ERROR] ",
        .warn => "[WARN] ",
        .info => "[INFO] ",
        .debug => "[DEBUG] ",
    };

    // Simple serial write (no formatting for now)
    stdio.string_writter(prefix);
    // const runtime_str: [*]const u8 = format;
    // string_writter(format);
    stdio.string_writter(format);

    // _ = runtime_str;
    // Could implement simple formatting here
    // _ = format;
    _ = args;
    // _ = scope;
}

// Simple panic handler without 32-bit relocations
pub fn kernel_panic_handler(msg: []const u8, first_trace_addr: ?usize) noreturn {
    const src = @src();
    _ = src;

    // Write to serial
    const panic_msg = "\nKERNEL PANIC: ";
    stdio.string_writter(panic_msg);
    stdio.string_writter(msg);

    if (first_trace_addr) |addr| {
        stdio.string_writter("\nError at ");
        stdio.print_hex(@intCast(addr));

        // const index: usize = 0;
        // var instruction_addresses: [50]usize = undefined;
        // var stack_trace: std.builtin.StackTrace = std.builtin.StackTrace{ .instruction_addresses = &instruction_addresses, .index = index };
        // std.debug.captureStackTrace(first_trace_addr, &stack_trace);
        // std.debug.dumpStackTrace(stack_trace);
    } else {
        stdio.string_writter("\nDoes not know error addr\n");
    }

    // std.debug.getSelfDebugInfo();

    // Halt
    while (true) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}
