const std = @import("std");

const ELFSectionHeader = struct {
    sh_name: u32,
    sh_type: u32,
    sh_flags: usize,
    sh_addr: usize,
    sh_offset: usize,
    sh_size: usize,
    sh_link: u32,
    sh_info: u32,
    sh_addralign: usize,
    sh_entsize: usize,
};

