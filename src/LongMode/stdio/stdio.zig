// stdio/stdio.zig

const std = @import("std");
const com1 = @import("com1.zig");
const string = @import("string");

pub const com1_write = com1.com1_write;
pub const com1_write_c = com1.com1_write_c;
pub const com1_write_ptr = com1.com1_write_ptr;

// use string_writter for every function implementation
pub const string_writter = com1_write;
// ========================== String transformation functions ========================

/// Worst case scenario, number buffer size
const wcs_buffer_size = 32;

// Helper: Write error message into buffer
fn write_error(buffer: []u8) []const u8 {
    const err_msg = "[ERR]";
    const len = @min(err_msg.len, buffer.len);
    @memcpy(buffer[0..len], err_msg[0..len]);
    return buffer[0..len];
}

pub fn snprint_int(number: i64, buffer: []u8) []const u8 {
    return std.fmt.bufPrint(buffer, "{}", .{number}) catch {
        // Return error indicator
        return write_error(buffer);
    };
}

pub fn snprint_uint(number: u64, buffer: []u8) []const u8 {
    return std.fmt.bufPrint(buffer, "{}", .{number}) catch {
        return write_error(buffer);
    };
}

pub fn snprint_float(number: f64, buffer: []u8) []const u8 {
    return std.fmt.bufPrint(buffer, "{d}", .{number}) catch {
        return write_error(buffer);
    };
}

pub fn snprint_string(str: []const u8, buffer: []u8) []const u8 {
    return std.fmt.bufPrint(buffer, "{s}", .{str}) catch {
        return write_error(buffer);
    };
}

pub fn snprint_char(char: u8, buffer: []u8) []const u8 {
    return std.fmt.bufPrint(buffer, "{c}", .{char}) catch {
        return write_error(buffer);
    };
}

pub fn print_int_simple(number: i64) void {
    var buffer: [wcs_buffer_size]u8 = undefined;

    const str = std.fmt.bufPrint(&buffer, "{}", .{number}) catch {
        string_writter("[int too large]");
        return;
    };
    string_writter(str);
}

pub fn print_int(number: i64) void {
    var buffer: [wcs_buffer_size]u8 = undefined;
    const str = snprint_int(number, &buffer);
    string_writter(str);
}

pub fn print_two_numbers(num_int: i64, num_uint: u64) void {
    var buffer: [2 * wcs_buffer_size]u8 = undefined;
    var pos: usize = 0;

    const str_int = snprint_int(num_int, &buffer);
    pos += str_int.len;
    const str_u64 = snprint_uint(num_uint, buffer[pos..]);

    string_writter(str_int);
    string_writter(" ");
    string_writter(str_u64);
    string_writter("\n");
}

// uint64 -> string
// int64 -> string
// float64 -> string
