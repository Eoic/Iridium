 // Headers and declarations
%{
    #include <stdio.h>
    #include "parser.tab.h"
    void yyerror(char*);
    int yyparse();
%}

 // Regular expressions
name            \w([0-9]\w)+
number          [0-9]+
float           [+-]?([0-9]*[.])?[0-9]+
whitespace      [\r\t\s]
linefeed        \n
string          \"([^\\\"]|\\.)*\"

 // Token specification
%%
"Int" | "Double" | "String" { yylval.dataType = strdup(yytext); return DATA_TYPE; }
{name}                      { return NAME; }
{number}                    { yylval.ival = atoi(yytext); return INT; }
{float}                     { yylval.fval = areturn FLOAT; }
{string}                    { return STRING; }
"{"                         { return CURLY_BRACKET_L; }
"}"                         { return CURLY_BRACKET_R; }
"["                         { return BOX_BRACKET_L; }
"]"                         { return BOX_BRACKET_R; }
">"                         { return GT; }
"<"                         { return LT; }
"="                         { return EQ; }
">="                        { return GTE; }
"<="                        { return LTE; }
"=="                        { return CMP; }
"&&"                        { return AND; }
"||"                        { return OR; }
"^"                         { return POWER_BY; }
"@"                         { return INVERSE; }
":"                         { return TYPE_ASSIGN; }
"->"                        { return METHOD_RETURN_TYPE; }
"("                         { return PARENTHESIS_L; }
")"                         { return PARENTHESIS_R; }
"+"                         { return PLUS; }
"-"                         { return MINUS; }
"*"                         { return MUL; }
"/"                         { return DIV; }
"%"                         { return MOD; }
","                         { return COMMA; }
"if"                        { return IF; }
"loop"                      { return LOOP; }
"until"                     { return UNTIL; }    
"void" 						{ return VOID; }         
"++" 						{ return INC_OP; }
"--" 						{ return DEC_OP; }
";"                         { return SEMICOLON; }
{whitespace}                ;
{linefeed}                  ++yylineno;
.                           yyterminate();
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