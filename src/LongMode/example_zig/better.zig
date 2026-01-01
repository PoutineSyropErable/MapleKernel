const std = @import("std");

// =======================
// Color struct (AARRGGBB)
// =======================
const Color = packed struct(u32) {
    b: u8, // bits 0-7
    g: u8, // bits 8-15
    r: u8, // bits 16-23
    a: u8, // bits 24-31

    pub fn raw(self: Color) u32 {
        return @as(u32, self);
    }
};

// =======================
// LVT Timer Register
// =======================
const DeliveryMode = enum(u3) {
    fixed = 0,
    lowest_priority = 1,
    smi = 2,
    nmi = 4,
    init = 5,
    extint = 7,
};

const TimerMode = enum(u2) {
    one_shot = 0,
    periodic = 1,
    tsc_deadline = 2,
};

const LvtTimer = packed struct(u32) {
    vector: u8, // bits 0-7
    delivery_mode: DeliveryMode, // bits 8-10
    _res1: u5, // bits 11-15 reserved
    mask: bool, // bit 16
    timer_mode: TimerMode, // bits 17-18
    _res2: u13, // bits 19-31 reserved

    pub fn raw(self: LvtTimer) u32 {
        return @as(u32, self);
    }

    pub fn init(vector: u8, delivery: DeliveryMode, mode: TimerMode, masked: bool) LvtTimer {
        return LvtTimer{
            .vector = vector,
            .delivery_mode = delivery,
            ._res1 = 0,
            .mask = masked,
            .timer_mode = mode,
            ._res2 = 0,
        };
    }
};

// =======================
// MMIO pointer macro-style helper
// =======================
pub fn mmioPtr(comptime addr: usize, comptime T: type) *volatile T {
    // return @as(*volatile T, @ptrFromInt(addr));

    return @ptrFromInt(addr);

    // // Alternative 3: If you want to be more explicit about the intermediate step
    // const ptr: *volatile type = @ptrFromInt(addr);
    // return @ptrCast(ptr);

    // Made useless from how good the new thing got
}

// =======================
// Example main
// =======================
pub fn main() void {
    std.debug.print("Hello world from zig, it will crash after this, as we deref a random address\n\n", .{});

    // --- Color example ---
    const fb_color: *volatile Color = @ptrFromInt(0xB000000);

    var c = fb_color.*;
    std.debug.print("Color before: R={}, G={}, B={}, A={}\n", .{ c.r, c.g, c.b, c.a });
    c.g = 0xFF;
    fb_color.* = c;
    // _ = std.debug.print("Color after:  R={}, G={}, B={}, A={}\n", .{ c.r, c.g, c.b, c.a });

    // --- LVT Timer example ---
    const lvt_timer = mmioPtr(0xFEE00320, LvtTimer); // LAPIC LVT Timer offset

    var timer = lvt_timer.*;
    // _ = std.debug.print("LVT Timer before: vector=0x{X}, delivery_mode={}, mask={}, timer_mode={}\n", .{ timer.vector, timer.delivery_mode, timer.mask, timer.timer_mode });

    // Modify LVT Timer
    timer.mask = true;
    timer.delivery_mode = .nmi;
    timer.timer_mode = .periodic;

    lvt_timer.* = timer;

    std.debug.print("LVT Timer after:  vector=0x{X}, delivery_mode={}, mask={}, timer_mode={}\n", .{ timer.vector, timer.delivery_mode, timer.mask, timer.timer_mode });
}
