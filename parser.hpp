/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENTIFIER = 258,
    INTEGER = 259,
    DOUBLE = 260,
    GT = 261,
    LT = 262,
    GTE = 263,
    LTE = 264,
    EQ = 265,
    NEQ = 266,
    ASSIGN = 267,
    PLUS_OP = 268,
    MINUS_OP = 269,
    MUL_OP = 270,
    DIV_OP = 271,
    MOD_OP = 272,
    INVERSE_OP = 273,
    POWER_OP = 274,
    INC_OP = 275,
    DEC_OP = 276,
    CURLY_BRACKET_L = 277,
    CURLY_BRACKET_R = 278,
    BOX_BRACKET_L = 279,
    BOX_BRACKET_R = 280,
    PAREN_L = 281,
    PAREN_R = 282,
    COMMA = 283,
    SEMICOLON = 284,
    AND = 285,
    OR = 286,
    TYPE_ASSIGN = 287,
    METHOD_RETURN_ARROW = 288,
    PLUS = 289,
    MINUS = 290,
    MUL = 291,
    DIV = 292,
    MOD = 293
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 12 "parser.y" /* yacc.c:1909  */

    Node *node;
    Block *block;
    Expression *expression;
    Statement *statement;
    Identifier *identifier;
    VariableDeclaration *var_declaration;
    std::vector <VariableDeclaration*> *variables;
    std::vector <Expression*> *expressions;
    std::string *string;
    int token;

#line 106 "parser.hpp" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
