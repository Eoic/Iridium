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
%token CURLY_BRACKET_LEFT CURLY_BRACKET_RIGHT
%token BOX_BRACKET_LEFT BOX_BRACKET_RIGHT

%%
E : INT '+' INT
  | INT '-' INT
  ;
%%
