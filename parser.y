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
%token NAME NUMBER                                      // Variable / function name
%token CURLY_BRACKET_L CURLY_BRACKET_R BOX_BRACKET_L    // Brackets and parentheses
       BOX_BRACKET_R PARENTHESIS_L PARENTHESIS_R COMMA
%token TYPE_ASSIGN METHOD_RETURN_TYPE                   // Types
%token PLUS MINUS MUL DIV MOD POWER_BY                  // Arithmetic
%token LT GT LTE GTE EQ CMP AND OR                      // Comparison and assignment
%token INVERSE                                          // Misc
%token IF LOOP                                          // Keywords

%%
start : { printf("Started.\n"); } 
    ;
expr: expr PLUS expr
    | expr MINUS expr
    | expr MUL expr
    | expr DIV expr
    ;
%%