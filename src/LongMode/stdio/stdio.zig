const std = @import("std");
const builtin = @import("builtin");
const intrinsics = @import("intrinsics");

const inb = intrinsics.inb;
const outb = intrinsics.outb;

const COM1_PORT: u16 = 0x3F8; // The only one we use for stdio
const COM2_PORT: u16 = 0x2F8;
const COM3_PORT: u16 = 0x3E8;
const COM4_PORT: u16 = 0x2E8;
const COM5_PORT: u16 = 0x5F8;
const COM6_PORT: u16 = 0x4F8;
const COM7_PORT: u16 = 0x5E8;
const COM8_PORT: u16 = 0x4E8;

const io_port_offset = enum(u8) {
    // With DLAB = 0
    receive_transmit_buffer = 0, // +0, read
    // transmit_buffer = 0, // +0, write
    interrupt_enable = 1, // +1, read/write

    // With DLAB = 1
    // divisor_lsb = 0, // +0, least significant byte
    // divisor_msb = 1, // +1, most significant byte

    // Other registers
    interrupt_id_fifo_ctrl = 2, // +2, read
    line_control = 3, // +3
    modem_control = 4, // +4
    line_status = 5, // +5
    modem_status = 6, // +6
    scratch = 7, // +7
};

const LineStatusRegister = packed struct(u8) {
    /// bit 0
    data_ready: u1,
    /// bit 1
    overrun_error: u1,
    /// bit 2
    parity_error: u1,
    /// bit 3
    framing_error: u1,
    /// bit 4
    break_indicator: u1,
    /// bit 5
    transmitter_holding_register_empty: u1,
    /// bit 6
    transmitter_empty: u1,
    /// bit 7
    impending_error: u1,
};

fn com1_can_write() bool {
    const com1_status_line_port: u16 = COM1_PORT + @intFromEnum(io_port_offset.line_status);
    const com1_status_line_raw: u8 = inb(com1_status_line_port);
    const com1_status_line: LineStatusRegister = @bitCast(com1_status_line_raw);
    const ret: bool = (com1_status_line.transmitter_holding_register_empty == 1);
    // This generate a shift. But I guess, if it's inlined, then the shift will be removed
    return ret;
}

fn com1_wait_till_can_write() void {
    while (!com1_can_write()) {}
}

fn com1_putchar(char: u8) void {
    com1_wait_till_can_write();
    outb(COM1_PORT, char);
}

// Write a slice of bytes
// default zig strings
pub fn com1_write(bytes: []const u8) void {
    for (bytes) |byte| {
        com1_putchar(byte);
    }
}

// For pointers with explicit length
pub fn com1_write_ptr(ptr: [*]const u8, len: usize) void {
    var i: usize = 0;
    while (i < len) : (i += 1) {
        com1_putchar(ptr[i]);
    }
}

pub fn com1_write_c_string(cstr: [*:0]const u8) void {
    var i: usize = 0;
    while (cstr[i] != 0) : (i += 1) {
        com1_putchar(cstr[i]);
    }
}

// ========================== String transformation functions ========================

// uint64 -> string
// int64 -> string
// float64 -> string
