#include <iostream>
#include "ast.h"
#include "generator.hpp"

extern Block *program;
extern int yyparse();

int main()
{
    yyparse();
    GeneratorContext *context = new GeneratorContext();
    context->compileModule(*program);
    context->runCode();
}