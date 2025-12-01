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

    // Add additional source files to the module
    // mod.addSourceFile("src/z_otherLang/zig/utils.zig");
    // mod.addSourceFile("src/z_otherLang/zig/drivers.zig");
    // Add all your Zig files...

    // Freestanding settings
    mod.red_zone = false;

    // Create static library using addLibrary (not addStaticLibrary)
    const lib = b.addLibrary(.{
        .name = "kernel_zig",
        .root_module = mod,
        .linkage = .static, // Explicitly static for embedded
    });
    lib.root_module.addIncludePath(b.path("../../stdio"));

    // Install the library
    b.installArtifact(lib);
}
