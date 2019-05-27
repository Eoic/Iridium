#include <iostream>
#include <unistd.h>
#include "ast.h"
#include "generator.hpp"

extern Block *program; // AST tree root node pointer
extern int yyparse();  // Builds parse tree
extern FILE *yyin;     // Input stream file pointer

bool checkFlags(int argCount, char **arguments, bool *verboseOutput, bool *compileToFile, std::string *fileName)
{
    for (int i = 0; i < argCount; i++){
        if (std::strcmp(arguments[i], "-v") == 0){
            *verboseOutput = true;
        }
        if (std::strcmp(arguments[i], "-o") == 0)
        {
            *compileToFile = true;
            if(arguments[i+1] != nullptr){
                *fileName = arguments[i+1];
                return true;    
            }
            else{
                std::cerr << "Provide a file name when using -o option!" << std::endl;
                return false;
            }
        }
    }
}

int main(int argCount, char **arguments)
{
    bool verboseOutput = false;
    bool compileToFile = false;
    std::string fileName;
    if(!checkFlags(argCount, arguments, &verboseOutput, &compileToFile, &fileName))
        return -1;
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
        if (compileToFile)
            context.compileToExecutable(fileName);
        else
            context.runCode(); 

    }
}