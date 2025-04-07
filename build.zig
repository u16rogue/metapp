const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const metapp = b.addStaticLibrary(.{
        .name = "metapp",
        .target = target,
        .optimize = optimize,
    });

    metapp.addIncludePath(b.path("include"));
    metapp.addCSourceFiles(.{
        .root = b.path("src"),
        .flags = &.{ "--std=c++20" },
        .files = &.{},
    });

    b.installArtifact(metapp);
}
