// build.zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{
        .default_target = .{
            .cpu_arch = .x86_64,
            .os_tag = .freestanding,
            // .abi = .none,
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

    const string_module = b.createModule(.{
        .root_source_file = b.path("stdlib/string.zig"),
        .target = target,
        .optimize = optimize,
    });

    const intrinsics_modules = b.createModule(.{
        .root_source_file = b.path("stdlib/intrinsics.zig"),
        .target = target,
        .optimize = optimize,
    });

    const debug_module = b.createModule(.{
        .root_source_file = b.path("stdlib/debug.zig"),
        .target = target,
        .optimize = optimize,
    });

    // ======================= The Full kernel

    const entire_kernel_module = b.createModule(.{
        .root_source_file = b.path("kernel64/kernel64.zig"), // Source file
        .target = target,
        .optimize = optimize,
    });

    // might be a cool feature (error tracing)
    // entire_kernel_module.error_tracing = true;

    // Create the kernel library (static archive)
    const kernel_lib = b.addLibrary(.{
        .name = "kernel64",
        .root_module = entire_kernel_module,
        .linkage = .static,
    });

    // Add the modules
    entire_kernel_module.addImport("stdio", stdio_module);
    // entire_kernel_module.addImport("debug", debug_module);
    // debug_module.addImport("stdio", stdio_module);
    _ = debug_module;
    stdio_module.addImport("intrinsics", intrinsics_modules);
    stdio_module.addImport("string", string_module);

    // Add Zig-specific flags for kernel
    // kernel_lib.root_module.omit_frame_pointer = false;
    if (optimize != .Debug) {
        kernel_lib.pie = false;
        kernel_lib.root_module.pic = false;
        kernel_lib.root_module.stack_check = false;
        kernel_lib.root_module.stack_protector = false;
    } else {
        kernel_lib.pie = true;
        kernel_lib.root_module.pic = true;
        kernel_lib.root_module.stack_check = true;
        kernel_lib.root_module.stack_protector = false; // no support for stack protection
    }
    // kernel_lib.root_module.red_zone = false;
    kernel_lib.root_module.code_model = .kernel;
    kernel_lib.root_module.link_libc = false;
    kernel_lib.root_module.link_libcpp = false;
    kernel_lib.root_module.dwarf_format = .@"64";

    // kernel_lib.root_module.unwind_tables = .none;
    // kernel_lib.link_eh_frame_hdr = false;
    // kernel_lib.root_module.

    // Add include directories (For C/Cpp headers only)
    kernel_lib.root_module.addIncludePath(b.path("../ProtectedMode/LongModePrep/"));

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

    // Optional: Add assembly files

    // ======================= Create a test runner step

    // This doesn't really work in freestanding
    // I'd have to learn how to do it with test_runner.
    // I might need a custom testing environment. Semi Hosting
    const kernel_tests = b.addTest(
        .{
            .root_module = entire_kernel_module,
            .name = "test",
            .emit_object = true,
            .test_runner = null,
            .filters = &[_][]const u8{},
        },
    );

    const stdio_tests = b.addTest(
        .{
            .root_module = stdio_module,
            .name = "test",
            .emit_object = true,
            .test_runner = null,
            .filters = &[_][]const u8{},
        },
    );

    const run_kernel_tests = b.addRunArtifact(kernel_tests);
    const run_stdio_tests = b.addRunArtifact(stdio_tests);

    const test_step = b.step("test", "Run all tests");
    test_step.dependOn(&run_kernel_tests.step);
    test_step.dependOn(&run_stdio_tests.step);

    // b.addSystemCommand();

    // ======================= Test steps for the user

    // Set output directory to match your bash script
    const build_dir_path_str = "../../build64";
    const build_dir = b.path(build_dir_path_str);
    _ = build_dir;
    // kernel_lib.setOutputDir(b.pathFromRoot(build_dir_path_str));

    // Install the library to the build directory
    b.installArtifact(kernel_lib);
}
