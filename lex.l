 // Headers and declarations
%{
    #include <string>
    #include <stdio.h>
    #include "ast.h"
    #include "parser.hpp"
    #define SAVE_VALUE yylval.string = new std::string(yytext, yyleng)
    #define SAVE_TOKEN(match) (yylval.token = match) 
    void yyerror(char const*);
    int yyparse();
    extern "C" int yywrap();
%}

 // Regular expressions
identifier      [a-zA-Z_][a-zA-Z0-9_]*
double          [+-]?([0-9]*[.])?[0-9]+
integer         [0-9]+
whitespace      [ \t\s]*
linefeed        [\n]

 // Token specification
%%
"if"                        { return SAVE_TOKEN(IF); }
"else"                      { return SAVE_TOKEN(ELSE); }
"elsif"                     { return SAVE_TOKEN(ELSE_IF); }
"function"                  { return SAVE_TOKEN(FUNCTION); }
"loop"                      { return SAVE_TOKEN(LOOP); }
"until"                     { return SAVE_TOKEN(UNTIL); }
"return"                    { return SAVE_TOKEN(RETURN); }
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
"|"                         { return SAVE_TOKEN(VERTICAL_BAR); }
";"                         { return SAVE_TOKEN(SEMICOLON); }
{linefeed}                  ++yylineno;
{whitespace}                ;
.                           yyterminate();
%%

 // Called if input could not be parsed
void yyerror(char const* string) {
    printf("Could not parse input: %s\n", string);
}

int yywrap() {
    std::cout << "No syntax errors detected." << std::endl;
    return 1;
}