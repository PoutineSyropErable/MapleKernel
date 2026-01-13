const std = @import("std");

extern fn com1_write(s: [*]const u8) void;
pub fn some_stdio_function() void {
    com1_write("Hey, got modules working properly\n");
}
