const std = @import("std");

/// Setup the include directory of the `add_to` module.
/// Requires you to resolve the metapp dependency and pass it to `mpp_dependency`.
///
/// metapp requires standard version of a minimum c++20. Source files that use `metapp` must
/// have the `--std=c++20` flag.
pub fn addIncludeDirsTo(mpp_dependency: *std.Build.Dependency, add_to: *std.Build.Module) void {
    add_to.addIncludePath(mpp_dependency.path("include/"));
}

pub const flags = .{ "--std=c++20" };

pub fn build(_: *std.Build) void {
    @panic("metapp for zig is not meant to be built and should act as a helper instead");
}
