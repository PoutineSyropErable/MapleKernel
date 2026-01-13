// build.zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{
        .default_target = .{
            .cpu_arch = .x86_64,
            .os_tag = .freestanding,
            .abi = .none,
        },
    });

    const optimize = b.standardOptimizeOption(.{
        .preferred_optimize_mode = .ReleaseSafe,
    });

    // ======================= Specific modules

    const stdio_module = b.createModule(.{
        .root_source_file = b.path("stdio/stdio.zig"),
        .target = target,
        .optimize = optimize,
    });

    // ======================= The Full kernel

    const entire_kernel_module = b.createModule(.{
        .root_source_file = b.path("kernel64/kernel64.zig"), // Source file
        .target = target,
        .optimize = optimize,
    });

    entire_kernel_module.addImport("stdio", stdio_module);

    // Create the kernel library (static archive)
    const kernel_lib = b.addLibrary(.{
        .name = "kernel64",
        .root_module = entire_kernel_module,
        .linkage = .static,
    });

    // Add include directories (For C/Cpp headers only)
    kernel_lib.root_module.addIncludePath(b.path("../ProtectedMode/LongModePrep/"));
    // The ones bellow aren't really useful
    kernel_lib.root_module.addIncludePath(b.path("."));
    kernel_lib.root_module.addIncludePath(b.path("kernel64"));
    kernel_lib.root_module.addIncludePath(b.path("stdio"));

    // kernel_lib.addAssemblyFile(b.path("kernel64/kernel64_boot.asm"));
    // kernel_lib.addAssemblyFile(b.path("kernel64/guards.asm"));

    // Add single C file (Not needed)
    const add_c_file: bool = false;
    if (add_c_file) {
        kernel_lib.root_module.addCSourceFile(.{
            .file = b.path("kernel64/com1.c"),
            .flags = &.{
                "-std=gnu23",
                "-ffreestanding",
                "-mcmodel=kernel",
                "-mno-red-zone",
                "-fno-stack-protector",
            },
        });
    }

    // Add Zig-specific flags for kernel
    kernel_lib.root_module.omit_frame_pointer = false;
    kernel_lib.pie = false;
    // kernel_lib.red_zone = false;
    // kernel_lib.stack_check = false;
    // kernel_lib.stack_protector = false;

    // Optional: Add assembly files

    // Set output directory to match your bash script
    const build_dir_path_str = "../../build64";
    const build_dir = b.path("build64");
    _ = build_dir;
    _ = build_dir_path_str;
    // kernel_lib.setOutputDir(b.pathFromRoot(build_dir_path_str));

    // Install the library to the build directory
    b.installArtifact(kernel_lib);
}
