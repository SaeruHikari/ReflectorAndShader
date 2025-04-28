#include <optional>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Path.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include "SSL/ShaderCompiler.hpp"
#include "ShaderASTConsumer.hpp"

namespace skr::SSL {

using namespace clang::tooling;
static llvm::cl::OptionCategory ToolOptionsCategory = llvm::cl::OptionCategory("SSL compiler options");   
struct ShaderCompilerImpl : public ShaderCompiler
{
public:
    ShaderCompilerImpl(int argc, const char **argv)
    {
        auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolOptionsCategory);
        if (!ExpectedParser)
        {
            llvm::errs() << ExpectedParser.takeError();
        }
        OptionsParser = std::move(ExpectedParser.get());
        tool.emplace(ClangTool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList()));
    }

    int Run() override
    {
        auto factory = newFrontendActionFactory<CompileFrontendAction>();
        return tool->run(factory.get());
    }

private:
    std::optional<CommonOptionsParser> OptionsParser;
    std::optional<ClangTool> tool;
};

ShaderCompiler* ShaderCompiler::Create(int argc, const char **argv)
{
    return new ShaderCompilerImpl(argc, argv);
}

void ShaderCompiler::Destroy(ShaderCompiler* compiler)
{
    delete compiler;
}

} // namespace skr::SSL