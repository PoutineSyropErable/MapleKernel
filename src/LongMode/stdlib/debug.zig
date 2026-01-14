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

// Symbols from linker script
extern var __debug_info_start: u8;
extern var __debug_info_end: u8;
extern var __debug_abbrev_start: u8;
extern var __debug_abbrev_end: u8;
extern var __debug_str_start: u8;
extern var __debug_str_end: u8;
extern var __debug_line_start: u8;
extern var __debug_line_end: u8;
extern var __debug_ranges_start: u8;
extern var __debug_ranges_end: u8;

pub fn print_addresses_raw() void {
    // Also print the value at that address. Do a qword deref

}

pub fn print_addresses() void {
    stdio.string_writter("\n=== Debug Section Addresses ===\n");

    // Helper to print address and first qword
    const print_section = struct {
        fn print(name: []const u8, start_ptr: *const u8, end_ptr: *const u8) void {
            const start_addr = @intFromPtr(start_ptr);
            const size = @intFromPtr(end_ptr) - start_addr;

            stdio.string_writter(name);
            stdio.string_writter(": 0x");
            stdio.print_hex(start_addr);
            stdio.string_writter(" - 0x");
            stdio.print_hex(@intFromPtr(end_ptr));
            stdio.string_writter(" (");

            // Print size in hex and decimal
            var buf: [20]u8 = undefined;
            const size_str = std.fmt.bufPrint(&buf, "0x{x} / {} bytes)", .{ size, size }) catch "?)";
            stdio.string_writter(size_str);
            stdio.string_writter("\n");

            // Print first 8 bytes if section exists
            if (size > 0) {
                stdio.string_writter("  First 8 bytes: ");

                // Cast to pointer-to-u64 and dereference
                const first_qword_ptr = @as(*const u64, @ptrCast(@alignCast(start_ptr)));
                const first_qword = first_qword_ptr.*;

                stdio.string_writter("0x");
                stdio.print_hex(first_qword);
                stdio.string_writter("\n");

                // Also print as individual bytes for DWARF magic detection
                stdio.string_writter("\n");
            } else {
                stdio.string_writter("  [Empty section]\n");
            }
            stdio.string_writter("\n");
        }
    }.print;

    // Print each section
    print_section(".debug_info", &__debug_info_start, &__debug_info_end);
    print_section(".debug_abbrev", &__debug_abbrev_start, &__debug_abbrev_end);
    print_section(".debug_str", &__debug_str_start, &__debug_str_end);
    print_section(".debug_line", &__debug_line_start, &__debug_line_end);
    print_section(".debug_ranges", &__debug_ranges_start, &__debug_ranges_end);

    // Special check for DWARF magic
    check_dwarf_magic();
}

fn check_dwarf_magic() void {
    stdio.string_writter("=== DWARF Magic Check ===\n");

    // DWARF .debug_info starts with unit length (32-bit or 64-bit)
    const info_start = @as([*]const u8, @ptrCast(&__debug_info_start));
    const size: u64 = @intFromPtr(&__debug_info_end) - @intFromPtr(&__debug_info_start);

    if (size >= 12) { // Need at least 12 bytes for DWARF header
        const first_word: u32 = std.mem.readInt(u32, info_start[0..4], .little);

        if (first_word == 0xFFFFFFFF) {
            // 64-bit DWARF
            const length64 = std.mem.readInt(u64, info_start[4..12], .little);
            stdio.string_writter("DWARF64: unit length = ");
            stdio.print_hex(length64);
            stdio.string_writter("\n");
        } else {
            // 32-bit DWARF
            stdio.string_writter("DWARF32: unit length = ");
            stdio.print_hex(first_word);
            stdio.string_writter("\n");
        }

        const version_offset: usize = if (first_word == 0xFFFFFFFF) 12 else 4;
        if (size >= version_offset + 2) {
            const version = std.mem.readInt(u16, info_start[version_offset..][0..2], .little);
            stdio.string_writter("Version: ");
            var buf: [10]u8 = undefined;
            const ver_str = std.fmt.bufPrint(&buf, "{}", .{version}) catch "?";
            stdio.string_writter(ver_str);
            stdio.string_writter("\n");
        }
    }

    // Check .debug_str for typical strings
    if (@intFromPtr(&__debug_str_end) - @intFromPtr(&__debug_str_start) > 0) {
        stdio.string_writter(".debug_str first string: ");

        // Print first null-terminated string
        var i: usize = 0;
        const str_start = @as([*]const u8, @ptrCast(&__debug_str_start));
        while (i < 100 and i < (@intFromPtr(&__debug_str_end) - @intFromPtr(&__debug_str_start))) {
            if (str_start[i] == 0) break;
            // Print character if printable
            if (str_start[i] >= 32 and str_start[i] < 127) {
                stdio.string_writter(&[_]u8{str_start[i]});
            } else {
                stdio.string_writter(".");
            }
            i += 1;
        }
        stdio.string_writter("\n");
    }
}

/// Print the i-th null-terminated string from .debug_str section
/// Index is zero-based (0 = first string, 1 = second string, etc.)
pub fn print_debug_str_string(index: usize) void {
    // Get boundaries from linker script
    const str_start = @as([*]const u8, @ptrCast(&__debug_str_start));
    const str_end = @as([*]const u8, @ptrCast(&__debug_str_end));
    const total_size = @intFromPtr(str_end) - @intFromPtr(str_start);

    if (total_size == 0) {
        stdio.string_writter(".debug_str is empty\n");
        return;
    }

    var current_string_index: usize = 0;
    var position: usize = 0;

    // Walk through the string table
    while (position < total_size) {
        // Find length of current string
        var len: usize = 0;
        while (position + len < total_size and str_start[position + len] != 0) {
            len += 1;
        }

        // Check if this is the string we're looking for
        if (current_string_index == index) {
            stdio.string_writter(".debug_str[");
            stdio.print_uint(index);
            stdio.string_writter("]: \"");

            // Print the string
            for (0..len) |j| {
                const ch = str_start[position + j];
                if (ch >= 32 and ch < 127) {
                    // Printable ASCII
                    stdio.string_writter(&[_]u8{ch});
                } else {
                    // Non-printable
                    stdio.string_writter(".");
                }
            }

            stdio.string_writter("\" (length=");
            stdio.print_uint(len);
            stdio.string_writter(")\n");
            return;
        }

        // Move to next string (skip null terminator)
        position += len + 1;
        current_string_index += 1;
    }

    // If we get here, index was out of bounds
    stdio.string_writter("String index ");
    stdio.print_uint(index);
    stdio.string_writter(" out of bounds (");
    stdio.print_uint(current_string_index);
    stdio.string_writter(" strings total)\n");
}
