%{
    #include <iostream>
    #include <stdio.h>
    #include <stdlib.h>
    #include "ast.h" 
    extern int yylex();
    extern void yyerror(char*);
    Block *programBlock;
%}

 // Token types
%union {
    Node *node;
    Block *block;
    Expression *expression;
    Statement *statement;
    Identifier *identifier;
    VariableDeclaration *varDeclaration;
    std::vector <VariableDeclaration> *variables;
    std::vector <Expression> *expressions;
    std::string *string;
    int token;
}

%token <string> IDENTIFIER INTEGER DOUBLE       // Variables
%token <token>  GT LT GTE LTE EQ NEQ ASSIGN     // Comparing
%token <token>  PLUS_OP MINUS_OP MUL_OP DIV_OP  // Arithmetic Operators
%token <token>  MOD_OP INVERSE_OP POWER_OP      
%token <token>  INC_OP DEC_OP                   
%token <token>  CURLY_BRACKET_L CURLY_BRACKET_R // Brackets and parentheses 
%token <token>  BOX_BRACKET_L BOX_BRACKET_R     
%token <token>  PAREN_L PAREN_R COMMA SEMICOLON
%token <token>  AND OR                          // Logical operators
%token <token>  TYPE_ASSIGN METHOD_RETURN_ARROW // Misc 

%type <identifier>  identifier
%type <expression>  numbers expression inversion
%type <variables>   function_arguments
%type <expressions> call_arguments
%type <block>       program statements block
%type <statement>   statement var_declaration fun_declaration
%type <token>       comparison

%left PLUS MINUS MUL DIV MOD                    // Operators associativity 

%start program

%%
program : statements { programBlock = $1; }
        ;

statements : statement            { $$ = new Block(); $$->statements.push_back($<statement>1); }
           | statements statement { $1->statements.push_back($<statement>2); }
           ;

statement : var_declaration | fun_declaration
          | expression { $$ = new ExpressionStatement(*$1); }
          ;

block : CURLY_BRACKET_L statements CURLY_BRACKET_R { $$ = $2; }
      | CURLY_BRACKET_L CURLY_BRACKET_R            { $$ = new Block(); }

var_declaration : identifier TYPE_ASSIGN identifier                   { $$ = new VariableDeclaration(*$3, *$1); }
                | identifier TYPE_ASSIGN identifier ASSIGN expression { $$ = new VariableDeclaration(*$3, *$1, $5); }
                ;

fun_declaration :                                          { $$ = new VariableList(); }
                | function_arguments COMMA var_declaration { $1->push_back($<var_declaration>3); }
                ;

function_arguments : { $$ = new VariableList(); }
                   | function_arguments COMMA var_declaration { $1->push_back($<var_declaration>3); } 

identifier : IDENTIFIER { $$ = new Identifier(*$1); delete $1; }
           ;

numbers : INTEGER { $$ = new INTEGER(atol($1->c_str())); delete $1; }
        | DOUBLE  { $$ = new DOUBLE(atof($1->c_str())); delete $1; }
        ;

expression : identifier ASSIGN expression              { $$ = new Assignment(*$<identifier>1, *$3); }
           | identifier PAREN_L call_arguments PAREN_R { $$ = new MethodCall(*$1, *$3); delete *$3; }
           | numbers
           | expression comparison expression          { $$ = new BinaryOperator(*$1, $2, *$3); }
           | PAREN_L expression PAREN_R                { $$ = $2; }
           ;

call_arguments :                                 { $$ = new ExpressionList(); }
               | expression                      { $$ = new ExpressionList(); $$->push_back($1); }
               | call_arguments COMMA expression { $1->push_back($3); }

inversion : INVERSE_OP identifier { $$ = new Inversion(*<identifier>2); }
          ;

comparison : LT 
           | GT 
           | LTE 
           | GTE 
           | EQ 
           | NEQ
           ;

%%