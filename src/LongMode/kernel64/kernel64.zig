// const std = @import("std");

const std = @import("std");
const builtin = @import("builtin");

// const file2 = @import("kernel64.file2.zig");
const stdio = @import("stdio");

// const std_options = @import("kernel64.std_options.zig");

// Include std_options to customize std

// Optional: Also export a direct panic function
// pub const panic = std.debug.FullPanic(std_options.kernel_panic_handler);

// Declare external C function for printing
extern fn com1_putc(c: u8) void;
extern fn com1_write(s: [*]const u8) void;

// Helper function to print null-terminated strings
fn print_str(str: [*]const u8) void {
    var i: usize = 0;
    while (str[i] != 0) {
        com1_putc(str[i]);
        i += 1;
    }
}

// Helper function to print with newline
fn println(str: [*]const u8) void {
    print_str(str);
    com1_putc('\n');
}

// Helper function to print address
fn print_addr(label: [*]const u8, addr: *const anyopaque) void {
    print_str(label);
    print_str(" at 0x");
    _ = addr;
    com1_putc('\n');
}

// ========== .rodata SECTION (read-only data) ==========
const rodata_string = "Hello from .rodata! This string is read-only.";
const rodata_number: u32 = 0xDEADBEEF;
const rodata_array = [_]u8{ 'Z', 'I', 'G', '!', 0 };

// ========== .data SECTION (initialized read-write data) ==========
var data_counter: u32 = 42;
var data_string = "Hello from .data! This string is read-write.";
var data_buffer = [_]u8{ 1, 2, 3, 4, 5 };

// ========== .bss SECTION (uninitialized data) ==========
var bss_buffer: [1024]u8 = undefined; // Will be zero-initialized
var bss_counter: u64 = undefined; // Will be zero
var bss_array: [100]u32 = undefined; // Will be zero

export fn kernel64_zig_main() noreturn {
    // Your kernel code here

    // Setup your kernel...

    // const value: i32 = file2.zig_add(5, 6);
    // _ = value + 1;

    // std_options.kernel_panic_handler("End of kernel\n", null);
    // std_options.kernel_log(.err, .default, "Some Panic Msg\n", .{});

    com1_write("\nHello from zig\n");
    const msg: []const u8 = "test other method\n";
    com1_write(msg.ptr);
    com1_write(data_string);

    // stdio.some_stdio_function();

    // Never return
    while (true) {
        // asm volatile ("mov rax, 15");
        asm volatile ("hlt");
    }
}
