// panic.zig
const std = @import("std");
const stdio = @import("stdio");
const builtin = @import("builtin");

pub fn panic(msg: []const u8, src: std.builtin.SourceLocation) noreturn {
    stdio.string_writter(msg);

    stdio.string_writter("\nThe module name: ");
    stdio.string_writter(src.module);
    stdio.string_writter("\nThe file name: ");
    stdio.string_writter(src.file);
    stdio.string_writter("\nThe function name: ");
    stdio.string_writter(src.fn_name);
    stdio.string_writter("\nThe Line: ");
    stdio.print_uint(src.line);
    stdio.string_writter("\nThe Column: ");
    stdio.print_uint(src.column);

    while (true) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}

pub inline fn assert(cond: bool, msg: []const u8, src: std.builtin.SourceLocation) void {
    comptime if (builtin.mode == .ReleaseFast or builtin.mode == .ReleaseSmall) {
        return;
    };
    if (!cond) {
        panic(msg, src);
        // passing src here cause @src.fn_name = assert
    }
}
