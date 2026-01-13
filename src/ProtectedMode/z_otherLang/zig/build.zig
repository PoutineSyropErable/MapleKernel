const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .x86,
        .os_tag = .freestanding,
    });

    const optimize = b.standardOptimizeOption(.{});

    // Create the module first
    const mod = b.createModule(.{
        .root_source_file = b.path("kernel_zig.zig"),
        .target = target,
        .optimize = optimize,
    });

    // Use addObject instead of addLibrary to get a .o file
    const obj = b.addObject(.{
        .name = "kernel_zig",
        .root_module = mod,
    });

    // This is just for C headers
    obj.root_module.addIncludePath(b.path("../../stdio"));
    obj.root_module.addIncludePath(b.path("../../stdlib/"));
    obj.root_module.red_zone = false;

    // Install the object file
    // CORRECT: Install object file using addInstallFile
    const install_obj = b.addInstallFile(obj.getEmittedBin(), "kernel_zig.o");
    b.getInstallStep().dependOn(&install_obj.step);
}
