// stdlib/string.zig
const std = @import("std");

/// Worst case scenario, number buffer size
pub const wcs_buffer_size = 32;

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

pub fn snprint_hex(number: u64, buffer: []u8) []const u8 {
    return std.fmt.bufPrint(buffer, "{X}", .{number}) catch {
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
