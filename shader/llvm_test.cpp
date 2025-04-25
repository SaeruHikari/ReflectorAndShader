#include "SSL/ShaderCompiler.hpp"

int main(int argc, const char **argv)
{
    auto compiler = skr::SSL::ShaderCompiler::Create(argc, argv);
    int exit_code = 0;
    if (compiler)
    {
        exit_code = compiler->Run();
        skr::SSL::ShaderCompiler::Destroy(compiler);
    }
    return exit_code;
}