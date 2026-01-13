// std_options.zig - Customize std for kernel
const std = @import("std");

extern fn com1_putc(c: u8) void;
extern fn com1_write(s: [*]const u8) void;

/// Customize standard library behavior for kernel
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
    comptime scope: @Type(.enum_literal),
    comptime format: []const u8,
    args: anytype,
) void {
    // Write to COM1 serial port
    const COM1 = 0x3F8;

    // Prefix with log level
    const prefix = switch (level) {
        .err => "[ERROR] ",
        .warn => "[WARN] ",
        .info => "[INFO] ",
        .debug => "[DEBUG] ",
    };

    // Simple serial write (no formatting for now)
    for (prefix) |c| {
        while ((@as(*volatile u8, @ptrFromInt(COM1 + 5)).* & 0x20) == 0) {}
        @as(*volatile u8, @ptrFromInt(COM1)).* = c;
    }

    // Could implement simple formatting here
    _ = format;
    _ = args;
    _ = scope;
}

// Create a wrapper that converts slice to C pointer
fn com1_write_slice(msg: []const u8) void {
    com1_write(msg.ptr); // .ptr gets the [*]const u8 pointer
}

// Simple panic handler without 32-bit relocations
pub fn kernel_panic_handler(msg: []const u8, first_trace_addr: ?usize) noreturn {
    _ = first_trace_addr;

    // Write to serial
    const panic_msg = "KERNEL PANIC: ";
    com1_write(panic_msg);
    com1_write(msg.ptr);

    // Halt
    while (true) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}
