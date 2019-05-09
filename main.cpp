#include <iostream>
#include "ast.h"

extern Block *program;
extern int yyparse();

int main()
{
    yyparse();
    std::cout << program << std::endl;
}