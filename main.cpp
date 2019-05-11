#include <iostream>
#include "ast.h"
#include "generator.hpp"

extern Block *program; // AST tree root node pointer
extern int yyparse();  // Builds parse tree
extern FILE *yyin;     // Input stream file pointer

void BindCoreFunctions(GeneratorContext *context)
{
    // TODO: Add print function
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
    GeneratorContext *context = new GeneratorContext(verboseOutput);
    BindCoreFunctions(context);
    context->compileModule(*program);
    context->runCode();
}