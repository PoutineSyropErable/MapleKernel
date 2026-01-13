pub fn inb(port: u16) u8 {
    var result: u8 = undefined;
    asm volatile ("inb %dx, %al"
        : [ret] "={al}" (result),
        : [p] "{dx}" (port),
        : .{ .rcx = true, .r11 = true, .memory = true });
    return result;
}

pub fn outb(port: u16, value: u8) void {
    asm volatile ("outb %al, %dx"
        :
        : [val] "{al}" (value),
          [p] "{dx}" (port),
        : .{ .rcx = true, .r11 = true, .memory = true });
}
