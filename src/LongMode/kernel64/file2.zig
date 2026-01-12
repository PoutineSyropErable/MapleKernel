// file2.zig - Simple helper functions

// Export the function so it's visible to other files
pub fn zig_add(a: i32, b: i32) i32 {
    return a + b;
}

// Another simple function
pub fn zig_multiply(a: i32, b: i32) i32 {
    return a * b;
}

// You can also export for C if needed
export fn zig_add_c(a: c_int, b: c_int) c_int {
    return a + b;
}
