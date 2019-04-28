%{
    #include <iostream>
    #include <stdio.h>
    #include <stdlib.h>
    extern int yylex();
    extern void yyerror(char*);
%}

 // Token types
%union {
    int ival;
    float fval;
    char* sval;
}

%token <ival> INT
%token <fval> FLOAT
%token <sval> STRING

 // Token declarations
%token NAME
%token CURLY_BRACKET_L CURLY_BRACKET_R BOX_BRACKET_L BOX_BRACKET_R PARENTHESIS_L PARENTHESIS_R
%token LT GT LTE GTE EQ CMP AND OR
%token POWER_BY
%token INVERSE
%token TYPE_ASSIGN METHOD_RETURN_TYPE

%%
E : INT '+' INT
  | INT '-' INT
  ;
%%
