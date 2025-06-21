#include <vector>
#include <string>
#include "SSL/ShaderCompiler.hpp"

#include "SSL/langs/HLSLGenerator.hpp"
#include "SSL/AST.hpp"

int main(int argc, const char **argv)
{
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i)
    {
        args.emplace_back(argv[i]);
    }
    args.emplace_back("--");
    args.emplace_back("-std=c++23");
    args.emplace_back("-fsyntax-only");
    // swizzle uses reference member in union
    args.emplace_back("-fms-extensions");
    args.emplace_back("-Wno-microsoft-union-member-reference");

    std::vector<const char*> args_ptr(args.size());
    for (size_t i = 0; i < args.size(); ++i)
    {
        args_ptr[i] = args[i].c_str();
    }
    
    auto compiler = skr::SSL::ShaderCompiler::Create(args_ptr.size(), args_ptr.data());
    int exit_code = 0;
    if (compiler)
    {
        exit_code = compiler->Run();
        const auto& AST = compiler->GetAST();

        skr::SSL::SourceBuilderNew sb;
        skr::SSL::HLSLGenerator hlsl_generator;
        std::wcout << hlsl_generator.generate_code(sb, AST) << std::endl;
        std::wcout << AST.dump() << std::endl;

        skr::SSL::ShaderCompiler::Destroy(compiler);
    }
    return exit_code;
}