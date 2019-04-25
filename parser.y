%{
    #include <stdio.h>
    #include <stdlib.h>
    extern int yylex();
    extern void yyerror(char*);
%}

 // Token types
%union {
    int intValue;
    float floatValue;
    char* stringValue;
}

 // Tokens
%token <stringValue> NAME
