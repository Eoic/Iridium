#include <iostream>
#include <unistd.h>
#include "ast.h"
#include "generator.hpp"

extern Block *program; // AST tree root node pointer
extern int yyparse();  // Builds parse tree
extern FILE *yyin;     // Input stream file pointer

void checkFlags(int argCount, char **arguments, bool *verboseOutput)
{
    for (int i = 0; i < argCount; i++)
        if (std::strcmp(arguments[i], "-v") == 0)
            *verboseOutput = true;
}

int main(int argCount, char **arguments)
{
    bool verboseOutput = false;
    checkFlags(argCount, arguments, &verboseOutput);
    GeneratorContext context(verboseOutput);

    // Invalid parameters
    if (argCount < 2)
        std::cerr << "Use: " << arguments[0] << " <program.ird>" << std::endl;

    // Compile given files
    for (int i = 1; i < argCount; i++)
    {
        if (std::strcmp(arguments[i], "-v") == 0)
            continue;

        std::cout << "-----------------------------------------------------------" << std::endl;
        std::cout << "Compiling source file: [" << arguments[i] << "]" << std::endl;

        if ((yyin = fopen(arguments[i], "r")) == 0)
        {
            std::cerr << "File " << arguments[i] << " does not exist." << std::endl;
            continue;
        }

        yyparse();
        context.compileModule(*program);
        context.runCode();
    }
}
