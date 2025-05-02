add_rules("mode.debug", "mode.release")

set_languages("c11", "cxx20")
set_policy("build.ccache", false)

if (is_os("windows")) then 

target("libTooling")
    set_kind("phony")
    -- add_cxflags("/GR-", {public=true, force=true, tools={"clang_cl", "cl"}})
    add_links("lib/**", {public=true})
    add_syslinks("Version", "ntdll", "Ws2_32", "advapi32", "Shcore", "user32", "shell32", "Ole32", {public = true})
    add_includedirs("include", {public=true})

else

add_requires("zstd")
target("libTooling")
    set_kind("phony")
    -- add_cxflags("/GR-", {public=true, force=true, tools={"clang_cl", "cl"}})
    add_syslinks("pthread", "curses", {public=true})
    add_linkdirs("lib", {public=true})
    add_includedirs("include", {public=true})
    add_packages("zstd", {public=true})
    on_load(function (target, opt)
        local libs = {}
        local p = "lib/lib*.a"
        for __, filepath in ipairs(os.files(p)) do
            local basename = path.basename(filepath)
            local matchname = string.match(basename, "lib(.*)$")
            table.insert(libs, matchname or basename)
        end
        target:add("links", libs, {public=true})
    end)
    
end

target("libTooling")
    add_cxflags("/GR-", {public=true, force=true, tools={"clang_cl", "cl"}})
    add_cxflags("-Wno-c++11-narrowing", "-fno-rtti", {public = true, force = true, tools={"gcc", "clang"}})

target("meta")
    set_kind("binary")
    set_runtimes("MD")  -- runtime depend on LLVM compiled version, official version is MT
    add_deps("libTooling")
    add_files("src/**.cpp")

includes("shader/xmake.lua")