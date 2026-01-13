// const std = @import("std");

const std = @import("std");
const builtin = @import("builtin");

const file2 = @import("file2.zig");
const stdio = @import("stdio");

const std_options = @import("std_options.zig");
const intrinsics = @import("intrinsics");

// Include std_options to customize std

// Optional: Also export a direct panic function
pub const panic = std.debug.FullPanic(std_options.kernel_panic_handler);

// ========== .rodata SECTION (read-only data) ==========
const rodata_string = "Hello from .rodata! This string is read-only.\n";
const rodata_number: u32 = 0xDEADBEEF;
const rodata_array = [_]u8{ 'Z', 'I', 'G', '!', 0 };

// ========== .data SECTION (initialized read-write data) ==========
var data_counter: u32 = 42;
var data_string = "Hello from .data! This string is read-write.\n";
var data_buffer = [_]u8{ 1, 2, 3, 4, 5 };

// ========== .bss SECTION (uninitialized data) ==========
var bss_buffer: [1024]u8 = undefined; // Will be zero-initialized
var bss_counter: u64 = undefined; // Will be zero
var bss_array: [100]u32 = undefined; // Will be zero

fn runtime_fail() void {
    var len: usize = 3;

    // Create a stack buffer as fallback
    var stack_buffer: [400]u8 = undefined;

    // Create a fixed buffer allocator using the stack buffer
    var fba = std.heap.FixedBufferAllocator.init(&stack_buffer);
    const allocator = fba.allocator();

    // Now allocate i32s from the byte allocator
    const ints = allocator.alloc(i32, len) catch {
        @panic("Allocation failed\n");
    };

    // Initialize values
    for (ints, 0..) |*item, i| {
        item.* = @as(i32, @intCast(i));
    }

    // Access element
    const a = ints[7];
    _ = a;
    len = 12;
}

export fn kernel64_zig_main() noreturn {
    // Your kernel code here

    // Setup your kernel...

    const value: i32 = file2.zig_add(5, 6);
    _ = value + 1;

    stdio.com1_write("\n\n=============== Start of Long Mode Zig Kernel ======================\n\n");
    stdio.com1_write("\nHello from zig\n");
    const msg: []const u8 = "test different ways to print (1: ptr, 2: regular)\n";
    const msg_c: [*:0]const u8 = "test different ways to print\n";
    stdio.com1_write_ptr(msg.ptr, msg.len);
    stdio.com1_write(msg);
    stdio.com1_write_c(msg_c);
    stdio.com1_write(data_string);

    // runtime_fail();

    stdio.com1_write("Zig write bytes\n");
    stdio.com1_write("Another so its not inlined\n");

    stdio.print_int(46);
    stdio.string_writter("\n");

    stdio.print_two_numbers(12345, 7890);

    std_options.kernel_log(.debug,
        // .default,
        "Some Panic Msg\n", .{});
    std_options.kernel_panic_handler("End of kernel\n", null);

    // Never return
    while (true) {
        // asm volatile ("mov rax, 15");
        asm volatile ("hlt");
    }
}
