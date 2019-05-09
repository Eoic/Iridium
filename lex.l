 // Headers and declarations
%{
    #include <string>
    #include <stdio.h>
    #include "ast.h"
    #include "parser.hpp"
    #define SAVE_VALUE yylval.string = new std::string(yytext, yyleng)
    #define SAVE_TOKEN(match) (yylval.token = match) 
    void yyerror(const char*);
    int yyparse();
    extern "C" int yywrap();
%}

 // Regular expressions
identifier      [a-zA-Z_][a-zA-Z0-9_]*
double          [0-9]+.[0-9]*
integer         [0-9]+
whitespace      [ \t\s]+
linefeed        [\n]+

 // Token specification
%%
{identifier}                { SAVE_VALUE; return IDENTIFIER; }
{integer}                   { SAVE_VALUE; return INTEGER; }
{double}                    { SAVE_VALUE; return DOUBLE; }
"{"                         { return SAVE_TOKEN(CURLY_BRACKET_L); }
"}"                         { return SAVE_TOKEN(CURLY_BRACKET_R); }
"["                         { return SAVE_TOKEN(BOX_BRACKET_L); }
"]"                         { return SAVE_TOKEN(BOX_BRACKET_R); }
">"                         { return SAVE_TOKEN(GT); }
"<"                         { return SAVE_TOKEN(LT); }
"=="                        { return SAVE_TOKEN(EQ); }
"!="                        { return SAVE_TOKEN(NEQ); }
">="                        { return SAVE_TOKEN(GTE); }
"<="                        { return SAVE_TOKEN(LTE); }
"="                         { return SAVE_TOKEN(ASSIGN); }
"&&"                        { return SAVE_TOKEN(AND); }
"||"                        { return SAVE_TOKEN(OR); }
"^"                         { return SAVE_TOKEN(POWER_OP); }
"@"                         { return SAVE_TOKEN(INVERSE_OP); }
":"                         { return SAVE_TOKEN(TYPE_ASSIGN); }
"->"                        { return SAVE_TOKEN(METHOD_RETURN_ARROW); }
"("                         { return SAVE_TOKEN(PAREN_L); }
")"                         { return SAVE_TOKEN(PAREN_R); }
"-"                         { return SAVE_TOKEN(MINUS_OP); }
"+"                         { return SAVE_TOKEN(PLUS_OP); }
"*"                         { return SAVE_TOKEN(MUL_OP); }
"/"                         { return SAVE_TOKEN(DIV_OP); }
"%"                         { return SAVE_TOKEN(MOD_OP); }
"++" 						{ return SAVE_TOKEN(INC_OP); }
"--" 						{ return SAVE_TOKEN(DEC_OP); }
","                         { return SAVE_TOKEN(COMMA); }
";"                         { return SAVE_TOKEN(SEMICOLON); }
{linefeed}                  ++yylineno;
{whitespace}                ;
.                           yyterminate();
%%

 // Called if input could not be parsed
void yyerror(const char* string) {
    printf("Could not parse input: %s\n", string);
}

int yywrap() {
    printf("Done parsing.");
    return 1;
}