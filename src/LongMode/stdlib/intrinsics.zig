// stdlib/intrinsics.zig
pub fn inb(port: u16) u8 {
    var result: u8 = undefined;
    asm volatile ("inb %%dx, %%al"
        : [ret] "={al}" (result),
        : [port] "{dx}" (port),
        : .{});
    return result;
}

pub fn outb(port: u16, value: u8) void {
    asm volatile ("outb %%al, %%dx"
        :
        : [val] "{al}" (value),
          [port] "{dx}" (port),
        : .{});
}
