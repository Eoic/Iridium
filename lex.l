 // Headers and declarations
 //#include "parser.tab.h"
%{
    #include <stdio.h>
    void yyerror(char*);
    int yyparse();
%}

 // Regular expressions
name            \w([0-9]\w)+
whitespace      [\r\t\s]
linefeed        \n

 // Token specification
%%
"Int" | "Double" | "String" { yylval.dataType = strdup(yytext); return DATA_TYPE; }
{name}                      { return NAME; }
"{"                         { return CURLY_BRACKET_LEFT; }
"}"                         { return CURLY_BRACKET_RIGHT; }
"["                         { return BOX_BRACKET_LEFT; }
"]"                         { return BOX_BRACKET_RIGHT; }
{whitespace}                ;
{linefeed}                  ++yylineno
.
%%

 // C
void yyerror(char* string) {
    printf("Could not parse input.");
}

 // Called once end of the input file is reached. 
int yywrap() {
    return 1;
}

int main() {
    yyparse();
}