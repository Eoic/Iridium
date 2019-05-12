#include <iostream>
#include "ast.h"
#include "generator.hpp"

extern Block *program; // AST tree root node pointer
extern int yyparse();  // Builds parse tree
extern FILE *yyin;     // Input stream file pointer

void BindCoreFunctions(GeneratorContext &context)
{
    //llvm::Constant *printFunction = context.module->getOrInsertFunction("printf", llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(llvmContext), llvm::PointerType::getInt8PtrTy(llvmContext), 0), true);
}

int main(int argCount, char **arguments)
{
    bool verboseOutput = false;

    if (argCount > 2)
    {
        if (std::strcmp(arguments[2], "-v") != 0)
        {
            std::cerr << "Use: " << arguments[0] << " <program.ird>" << std::endl;
            return 1;
        }

        verboseOutput = true;
    }

    // Set input stream for parser
    if ((yyin = fopen(arguments[1], "r")) == 0)
    {
        std::cerr << "Could not find given file " << arguments[1] << std::endl;
        return 1;
    }

    yyparse();
    GeneratorContext context(verboseOutput);// = new GeneratorContext(verboseOutput);
    BindCoreFunctions(context);
    context.compileModule(*program);
    context.runCode();
}