const std = @import("std");

const DebugLineHeader = packed struct {
    length: u32,
    version: u16,
    header_length: u32,
    min_instruction_length: u8,
    default_is_stmt: u8,
    line_base: i8,
    line_range: u8,
    opcode_base: u8,
    std_opcode_lengths: [12]u8,
};
