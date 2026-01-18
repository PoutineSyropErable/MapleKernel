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

fn print_int_simple(number: i64) void {
    var buffer: [string.wcs_buffer_size]u8 = undefined;

    const str = std.fmt.bufPrint(&buffer, "{}", .{number}) catch {
        string_writter("[int too large]");
        return;
    };
    string_writter(str);
}

fn print_int(number: i64) void {
    var buffer: [string.wcs_buffer_size]u8 = undefined;
    const str = string.snprint_int(number, &buffer);
    string_writter(str);
}

fn print_uint(number: u64) void {
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

fn print_two_numbers(num_int: i64, num_uint: u64) void {
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

fn snprintf_custom(buffer: []u8, comptime fmt: []const u8, args: anytype) []const u8 {
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

pub fn snprintf(buffer: []u8, comptime fmt: []const u8, args: anytype) []const u8 {
    const result = std.fmt.bufPrint(buffer, fmt, args) catch {
        return buffer[0..0]; // formatting failed or buffer too small
    };
    return result;
}

pub fn printf(comptime fmt: []const u8, args: anytype) void {
    // 1. Calculate required buffer size
    const STACK_BUFFER_SIZE = 256;

    const call_count = false;
    if (call_count) {
        // std.fmt.count will cause relocations errors
        const needed_len: usize = std.fmt.count(fmt, args);
        if (needed_len > STACK_BUFFER_SIZE) {
            string_writter("Needed len: ");
            print_uint(needed_len);
            string_writter("Is longer then: ");
            print_uint(STACK_BUFFER_SIZE);
            string_writter("\n");
            @panic("Stopping now\n");
        }
    }

    var stack_buf: [STACK_BUFFER_SIZE]u8 = undefined;
    const written = snprintf(&stack_buf, fmt, args);
    string_writter(written);
}

// ===========================================================================
// ZIG FORMATTING SPECIFIERS QUICK REFERENCE
// ===========================================================================
//
// ARGUMENT PASSING:
// - ALWAYS pass args as a tuple with .{}, even for single values:
//   print("Value: {}", .{x})      // ✅ Correct
//   print("Value: {}", x)         // ❌ WRONG - compilation error
//
// ================================ INTEGER TYPES =============================
// i8, i16, i32, i64, i128, isize      - Signed integers
// u8, u16, u32, u64, u128, usize      - Unsigned integers
// comptime_int                        - Compile-time integers
//
// INTEGER FORMAT SPECIFIERS:
// {}  or {d}   - Decimal (base 10)          Example: -42, 255
// {b}          - Binary (base 2)            Example: 101010, 0b1111
// {o}          - Octal (base 8)             Example: 52, 0o777
// {x}          - Hexadecimal lowercase      Example: 2a, 0xff
// {X}          - Hexadecimal uppercase      Example: 2A, 0xFF
//
// INTEGER FORMATTING OPTIONS (after colon):
// {x:0>4}      - Pad to width 4 with zeros: 0x002A
// {x: >8}      - Right-align width 8: "      2a"
// {x:<8}       - Left-align width 8:  "2a      "
// {x:^8}       - Center width 8:      "   2a   "
// {x:0>8X}     - Combined: uppercase, 0-padded, width 8
//
// ================================ FLOATING TYPES ============================
// f16, f32, f64, f128, f80                - Floating point numbers
// comptime_float                          - Compile-time floats
//
// FLOAT FORMAT SPECIFIERS:
// {}  or {e}   - Scientific notation       Example: 3.14e+00
// {d}          - Decimal notation          Example: 3.14159
//
// FLOAT FORMATTING OPTIONS:
// {:.2}        - 2 decimal places          Example: 3.14
// {:8.2}       - Width 8, 2 decimals       Example: "    3.14"
// {:.3e}       - 3 decimal places, scientific
//
// ================================ OTHER TYPES ===============================
// []const u8, []u8       - {s}            String slices     "Hello"
// [:0]const u8           - {s}            Null-terminated   "World"
// *T, ?*T                - {*}            Pointer           0x7fff...
// ?T                     - {?}            Optional          null or value
// anytype                - {any}          Debug any type    Complex debug
// char                   - {c}            Character         'A'
// bool                   - { }            Boolean           true, false
// enum                   - { }            Enum value        .variant
//
// ============================= SPECIAL SYNTAX ==============================
// POSITIONAL ARGUMENTS:
// "{0} {1} {0}", .{a, b}      - Reuse arguments by position
//
// NAMED ARGUMENTS (struct):
// "Name: {name}, Age: {age}", .{.name = "Bob", .age = 30}
//
// POINTER PREFIXES:
// "{0x:}", .{addr}       - Add 0x prefix to hex: 0x2A
// "{0o:}", .{num}        - Add 0o prefix to octal: 0o52
// "{0b:}", .{num}        - Add 0b prefix to binary: 0b1010
//
// ============================= KERNEL EXAMPLES =============================
// // Serial output
// serial.print("CPU: {}, Freq: {d} MHz\n", .{cpu_id, 3600});
// serial.print("Addr: {0x:0>16X}\n", .{0xFFFF_8000_0000});
//
// // Panic/error messages
// panic("Page fault at {0x:}, code: {x}\n", .{cr2, error_code});
//
// // Memory debugging
// debug.print("Alloc {d} bytes at {0x:}, remaining: {}\n",
//            .{size, ptr, mem_free});
//
// // Register dumps
// debug.print("RAX={0x:0>16X} RBX={0x:0>16X}\n", .{rax, rbx});
//
// =========================== COMMON ERRORS ================================
// ERROR: expected tuple or struct argument, found comptime_int
// CAUSE: print("Value: {}", 42);           // Missing .{}
// FIX:   print("Value: {}", .{42});        // Add .{}
//
// ERROR: unused argument
// CAUSE: print("Text");                    // Missing .{} entirely
// FIX:   print("Text", .{});               // Add empty tuple
//
// ERROR: cannot format array
// CAUSE: print("Arr: {}", .{arr});         // Array needs {any} or loop
// FIX:   print("Arr: {any}", .{arr});      // Use debug format
//
// ===========================================================================
// END OF FORMATTING REFERENCE
// ===========================================================================
