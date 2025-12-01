// Freestanding, no std
const c = @cImport({
    @cInclude("stdio.h"); // path relative to include dirs or use -I with zig build
});

pub fn zig_main() void {
    // Just an example computation
    var x: u32 = 42;
    x += 1;

    // You can later return values or modify memory, call C functions, etc.
    // _ = x; // avoid "unused variable" warning
    c.kprintf2("Hello from Zig!\n");
}
