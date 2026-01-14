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

pub fn print_int_simple(number: i64) void {
    var buffer: [string.wcs_buffer_size]u8 = undefined;

    const str = std.fmt.bufPrint(&buffer, "{}", .{number}) catch {
        string_writter("[int too large]");
        return;
    };
    string_writter(str);
}

pub fn print_int(number: i64) void {
    var buffer: [string.wcs_buffer_size]u8 = undefined;
    const str = string.snprint_int(number, &buffer);
    string_writter(str);
}

pub fn print_uint(number: u64) void {
    var buffer: [string.wcs_buffer_size]u8 = undefined;
    const str = string.snprint_uint(number, &buffer);
    string_writter(str);
}

pub fn print_hex(number: u64) void {
    var buffer: [string.wcs_buffer_size]u8 = undefined;
    const str = string.snprint_hex(number, &buffer);
    string_writter(str);
}

test "stdio_test" {
    try std.testing.expect(1 + 1 != 3);
    string_writter("Addition works test\n");
}

pub fn print_two_numbers(num_int: i64, num_uint: u64) void {
    var buffer: [2 * string.wcs_buffer_size]u8 = undefined;
    var pos: usize = 0;

    const str_int = string.snprint_int(num_int, &buffer);
    pos += str_int.len;
    const str_u64 = string.snprint_uint(num_uint, buffer[pos..]);

    string_writter(str_int);
    string_writter(" ");
    string_writter(str_u64);
    string_writter("\n");
}

const format = enum(u8) {
    NONE,
    int,
    uint,
    ptr,
    bits,
    float,
    float_scientific,
    string,
    char,
};

const format_meta = enum(u8) {
    NONE,
    int,
    uint,
    float,
    string,
    char,
};

fn parse_format(comptime format_string: []const u8) []const format {
    _ = format_string;
}

pub fn snprintf(buffer: []u8, comptime fmt: []const u8, args: anytype) []const u8 {
    //

    const formats_of_args = parse_format(fmt);
    var pos: usize = 0;
    pos = 2;

    _ = buffer;

    const ArgsType = @TypeOf(args);
    const arg_count = @typeInfo(ArgsType).Struct.fields.len;
    if (formats_of_args.len != arg_count) {
        @compileError("Wrong number of variable");
    }
    for (0..formats_of_args.len) |i| {
        const arg_format = formats_of_args[i];
        const arg_type = @TypeOf(args[i]);

        if (arg_format != .NONE) {
            const arg_format_type = switch (arg_format) {
                .int => i64,
                .uint => u64,
                .float => f64,
                .float_scientific => f64,
                .char => u8,
                .string => []const u8,
            };
            if (arg_type != arg_format_type) {
                @compileError("Gave a type, and it was wrong\n");
            }
        } else {}
    }
}

// uint64 -> string
// int64 -> string
// float64 -> string
