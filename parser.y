%{
    #include <stdio.h>
    #include <stdlib.h>
    int yylex ();
    void yyerror (char const*)
}%

 // Accepted tokens
%token NAME LEFT_BRACE RIGHT_BRACE