target("SSLAST")
    set_kind("static")
    set_runtimes("MD")  -- runtime depend on LLVM compiled version, official version is MT
    set_languages("cxx20")
    add_includedirs("AST/include", {public = true})
    add_files("AST/**.cpp")
    add_includedirs("AST/double-conversion", {public = true})
    add_files("AST/double-conversion/**.cc")

target("SSLLLVM")
    set_kind("static")
    set_runtimes("MD")  -- runtime depend on LLVM compiled version, official version is MT
    set_languages("cxx20")
    add_deps("libTooling", "SSLAST", {public = true})
    add_includedirs("LLVM/include", {public = true})
    add_files("LLVM/src/**.cpp")

target("ShaderTest")
    set_kind("binary")
    set_runtimes("MD")  -- runtime depend on LLVM compiled version, official version is MT
    set_languages("cxx20")
    add_deps("SSLAST")
    add_files("ast_test.cpp")

target("LLVMTest")
    set_kind("binary")
    set_runtimes("MD")  -- runtime depend on LLVM compiled version, official version is MT
    set_languages("cxx20")
    add_deps("SSLLLVM")
    add_files("llvm_test.cpp")